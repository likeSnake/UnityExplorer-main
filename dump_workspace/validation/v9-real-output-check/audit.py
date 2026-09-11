"""Read-only consistency audit for a SuperDumper output and its companion indexes."""
import collections
import csv
import hashlib
import json
from pathlib import Path
import re
import sys

dump = Path(sys.argv[1]).resolve()
folder = dump.with_name(dump.stem + '_structures')
manifest = (folder / 'export_manifest.txt').read_text(encoding='utf-8-sig')
base = int(re.search(r'模块基址：(0x[0-9A-Fa-f]+)', manifest)[1], 16)
size = int(re.search(r'模块大小：(0x[0-9A-Fa-f]+)', manifest)[1], 16)
errors = collections.Counter()
stats = {}


def check(condition, label):
    if not condition:
        errors[label] += 1


def rows(name):
    path = folder / name
    with path.open(encoding='utf-8-sig', newline='') as f:
        reader = csv.DictReader(f, delimiter='\t')
        count = 0
        for row in reader:
            check(None not in row and None not in row.values(), 'TSV columns:' + name)
            count += 1
            yield row
    match = re.search(r'^' + re.escape(name) + r'\t(\d+)\t(\d+)\t成功$', manifest, re.M)
    check(bool(match) and int(match[1]) == count and int(match[2]) == path.stat().st_size,
          'manifest:' + name)
    stats[name] = {'rows': count, 'bytes': path.stat().st_size}


classes = list(rows('class_meta.tsv'))
by_id = {r['ClassId']: r for r in classes}
by_va = {r['ClassVA']: r for r in classes}
check(len(by_id) == len(classes) == len(by_va), 'class uniqueness')
body_counts = {k: collections.Counter() for k in by_id}
methods = collections.Counter()
method_counts = collections.Counter()
address_counts = collections.Counter()
rva_counts = collections.Counter()
previous_sort = (-1, -1)
for r in rows('rva_map.tsv'):
    cid = r['ClassId']
    check(cid in by_id, 'method owner')
    c = by_id.get(cid, {})
    check(all(r[k] == c.get(v) for k, v in [('Assembly', 'Assembly'), ('Namespace', 'Namespace'), ('Class', 'Name')]),
          'method owner names')
    va = int(r['MethodVA'], 16)
    kind = 0 if base <= va < base + size else (1 if va else 2)
    check(previous_sort <= (kind, va), 'method sorting')
    previous_sort = (kind, va)
    check((r['RVA'] and int(r['RVA'], 16) == va - base) if kind == 0 else not r['RVA'], 'method RVA')
    address_counts[kind] += 1
    if kind == 0:
        rva_counts[va - base] += 1
    methods[(cid, va, r['Slot'], r['Signature'])] += 1
    method_counts[cid] += 1
interfaces = collections.defaultdict(list)
interface_status = collections.Counter()
interface_parity = collections.Counter()
interface_kinds = collections.Counter()
for r in rows('interfaces.txt'):
    cid = r['ClassId']
    check(cid in by_id, 'interface owner')
    check(r['ClassVA'] == by_id[cid]['ClassVA'], 'interface owner VA')
    interfaces[cid].append(r['Interface'])
    interface_status[r['Status']] += 1
    interface_parity[int(r['ArrayIndex']) % 2] += 1
    target = by_va.get(r['InterfaceClassVA'])
    interface_kinds[target['Kind'] if target else 'not_enumerated_definition'] += 1
    if target:
        check(target['Kind'] == 'interface', 'non-interface relation')

slots = set()
selected = set()
matched = collections.Counter()
unmatched = set()
for r in rows('all_bases.txt'):
    cid = r['ClassId']
    check(cid in by_id and r['ClassVA'] == by_id[cid]['ClassVA'], 'base owner')
    if r['SlotVA']:
        va = int(r['SlotVA'], 16)
        check(va not in slots, 'duplicate slot')
        check(base <= va <= base + size - 8 and va % 8 == 0, 'slot bounds/alignment')
        check(int(r['RVA'], 16) == va - base, 'slot RVA')
        slots.add(va)
        matched[cid] += 1
        if r['SelectedByDump'] == '1':
            selected.add((cid, va - base))
    else:
        unmatched.add(cid)
check(not (set(matched) & unmatched), 'matched/unmatched overlap')
check(len(matched) + len(unmatched) == len(classes), 'base coverage')

diagnostic_totals = collections.defaultdict(collections.Counter)
diagnostic_ids = set()
rejection_keys = ['NullArray', 'Unreadable', 'InvalidName', 'InvalidUtf8', 'OwnerMismatch',
                  'InvalidType', 'MissingAccessor', 'NotInterface']
for r in rows('member_diagnostics.tsv'):
    cid, category = r['ClassId'], r['Category']
    check(cid in by_id, 'diagnostic owner')
    check((cid, category) not in diagnostic_ids, 'duplicate diagnostic category')
    diagnostic_ids.add((cid, category))
    count = sum(int(r[k]) for k in rejection_keys)
    check(int(r['RawCount']) == int(r['Exported']) + int(r['LimitSkipped']) + int(r['Filtered']) + count,
          'diagnostic accounting')
    kind = {'字段': 'Field', '方法': 'Method', '属性': 'Property', '接口': 'Interface'}[category]
    check(int(r['Exported']) == int(by_id[cid][kind + 'Count']), 'diagnostic exported')
    check(int(r['RawCount']) == int(by_id[cid]['Raw' + kind + 'Count']), 'diagnostic raw')
    diagnostic_totals[category].update({k: int(r[k]) for k in rejection_keys + ['LimitSkipped']})
    diagnostic_totals[category]['groups'] += 1

method_line = re.compile(r'\t// RVA: 0x([0-9a-fA-F]*) VA: 0x([0-9a-fA-F]+)(?: Slot: (\d+))?$')
class_index = -1
cid = None
pending_method = None
section = None
line_count = 0
offset_status = collections.Counter()
body_selected = set()
with dump.open(encoding='utf-8') as source:
    for line_count, line in enumerate(source, 1):
        line = line.rstrip('\r\n')
        if line.startswith('// Offset::'):
            m = re.search(r'status=([A-Z_]+)', line)
            if m:
                offset_status[m[1]] += 1
        if line.startswith('// Class: '):
            class_index += 1
            check(pending_method is None, 'unfinished method')
            c = classes[class_index]
            cid = c['ClassId']
            section = None
            check(line[len('// Class: '):].strip() == c['Name'], 'body class name')
        elif cid and line.startswith('// Namespace: '):
            check(line[len('// Namespace: '):] == by_id[cid]['Namespace'], 'body namespace')
        elif cid and line.startswith('// Instance: '):
            check(int(line.split(': ', 1)[1], 16) == int(by_id[cid]['ClassVA'], 16), 'body class VA')
        elif cid and line.startswith('// RVA: 0x'):
            body_selected.add((cid, int(line.split(': ', 1)[1], 16)))
        elif cid and ' // TypeDefIndex: ' in line:
            declaration, index = line.rsplit(' // TypeDefIndex: ', 1)
            check(int(index, 16) == int(by_id[cid]['DumpTypeDefIndex'], 16), 'body type index')
            expected = interfaces[cid]
            if expected:
                check(' : ' in declaration and declaration.split(' : ', 1)[1].endswith(', '.join(expected)),
                      'body interface declaration')
        elif cid and line == '\t// Fields':
            section = 'Field'
        elif cid and line == '\t// Properties':
            section = 'Property'
        elif cid and line.startswith('\t// Methods:'):
            section = 'Method'
        elif cid and (m := method_line.fullmatch(line)):
            pending_method = (int(m[2], 16), m[3] or '')
        elif cid and pending_method is not None and line.startswith('\t') and not line.startswith('\t//'):
            va, slot = pending_method
            key = (cid, va, slot, line[1:].removesuffix(' { }'))
            check(methods[key] > 0, 'body method missing from index')
            methods[key] -= 1
            body_counts[cid]['Method'] += 1
            pending_method = None
        elif cid and section == 'Field' and line.startswith('\t') and '; // ' in line:
            body_counts[cid]['Field'] += 1
        elif cid and section == 'Property' and line.startswith('\t') and '{ ' in line:
            body_counts[cid]['Property'] += 1
check(class_index + 1 == len(classes), 'body class count')
check(all(n == 0 for n in methods.values()), 'extra indexed methods')
check(selected == body_selected, 'selected body slots')
for cid, c in by_id.items():
    for kind in ('Field', 'Method', 'Property'):
        check(body_counts[cid][kind] == int(c[kind + 'Count']), 'body count:' + kind)
    check(len(interfaces[cid]) == int(c['InterfaceCount']), 'interface count')

stats.update({
    'dump': {'name': dump.name, 'bytes': dump.stat().st_size, 'lines': line_count,
             'sha256': hashlib.file_digest(dump.open('rb'), 'sha256').hexdigest().upper()},
    'body_counts': {k: sum(c[k] for c in body_counts.values()) for k in ('Field', 'Method', 'Property')},
    'method_addresses': dict(address_counts), 'unique_rvas': len(rva_counts),
    'shared_rvas': sum(n > 1 for n in rva_counts.values()),
    'interface_status': dict(interface_status), 'interface_index_parity': dict(interface_parity),
    'interface_target_kinds': dict(interface_kinds),
    'slots': {'matched': len(slots), 'classes': len(matched), 'unmatched_classes': len(unmatched),
              'multiple_classes': sum(n > 1 for n in matched.values()), 'selected': len(selected)},
    'diagnostic_totals': dict(diagnostic_totals), 'offset_status': dict(offset_status),
    'consistency_errors': dict(errors),
    'scope': 'Consistency and strict UTF-8 only; semantic findings require review of diagnostics.'
})
print(json.dumps(stats, ensure_ascii=False, indent=2))
sys.exit(bool(errors))
