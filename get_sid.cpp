uint64_t merge_uid(uint32_t uid_low, uint32_t uid_high)
{
	uint64_t merged_uid = ((uint64_t)uid_high << 32) | uid_low;
	return merged_uid;
}

uint64_t Entity::getSID()
{
	// eadp_uid
	// uint32_t uida = *(uint32_t*)(buffer + OFFSET_m_platformUserId+16);
	// uint32_t uidb = *(uint32_t*)(buffer + OFFSET_m_platformUserId+20);
	// uint64_t eadp_uid = merge_uid(uida,uidb);

	// platform_uid
	uint32_t uida = *(uint32_t *)(buffer + OFFSET_m_platformUserId + 0);
	uint32_t uidb = *(uint32_t *)(buffer + OFFSET_m_platformUserId + 4);
	uint64_t platform_uid = merge_uid(uida, uidb);

	return platform_uid; //^ 0xDEADBEEF1B4DF00D;
}
