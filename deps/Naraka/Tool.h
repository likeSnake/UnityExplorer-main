#pragma once

namespace Tool
{
	//矩阵
	inline uintptr_t GetMatrixAddr(uintptr_t temp)
	{
		if (!temp) {
			return 0;
		}
		temp = mem.Read<uint64_t>(temp + 0x40);
		if (!temp) {
			return 0;
		}
		temp = mem.Read<uint64_t>(temp + 0x38);
		if (!temp) {
			return 0;
		}
		temp = mem.Read<uint64_t>(temp + 0x10);
		if (!temp) {
			return 0;
		}
		return temp + 0xdc;
	}

	//是否手持武器
	inline bool IsMeleeWeapon(WeaponType ObjWeaponType)
	{
		if (ObjWeaponType == WeaponType::Blade || ObjWeaponType == WeaponType::Dagge ||
			ObjWeaponType == WeaponType::Katana || ObjWeaponType == WeaponType::nunchucks ||
			ObjWeaponType == WeaponType::Spear || ObjWeaponType == WeaponType::Sw ||
			ObjWeaponType == WeaponType::twinblades || ObjWeaponType == WeaponType::rod ||
			ObjWeaponType == WeaponType::saber || ObjWeaponType == WeaponType::鞭子 ||
			ObjWeaponType == WeaponType::dualhalberd || ObjWeaponType == WeaponType::fan ||
			ObjWeaponType == WeaponType::hengdao || ObjWeaponType == WeaponType::punch ||
			ObjWeaponType == WeaponType::Knife || ObjWeaponType == WeaponType::lj)
		{
			return true;
		}
		return false;
	}
	//是否手持远程武器
	inline bool IsMeleeWeaponY(WeaponType ObjWeaponType)
	{
		if (ObjWeaponType == WeaponType::鸟铳 || ObjWeaponType == WeaponType::连弩 ||
			ObjWeaponType == WeaponType::神妙机甲 || ObjWeaponType == WeaponType::激光 ||
			ObjWeaponType == WeaponType::弓箭 || ObjWeaponType == WeaponType::大佛手 ||
			ObjWeaponType == WeaponType::喷火筒1 || ObjWeaponType == WeaponType::五眼铳 ||
			ObjWeaponType == WeaponType::三娘水矛 || ObjWeaponType == WeaponType::一窝蜂 ||
			ObjWeaponType == WeaponType::火炮 || ObjWeaponType == WeaponType::鞭子 ||
			ObjWeaponType == WeaponType::hadidazhao || ObjWeaponType == WeaponType::激光)
		{
			return true;
		}
		return false;
	}

}

