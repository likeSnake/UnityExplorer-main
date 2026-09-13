# Runtime information bundle

This directory contains structured hero and action information associated with:

- dump file: $sourceName
- dump SHA-256: $sourceHash
- catalog source: $catalogPath

The original dump file was not modified. heroes.json records the source kind and
the limitations of the snapshot. Unknown runtime/open-state/action-hash values stay
null. Verify every file against undle_manifest.json before consuming it.