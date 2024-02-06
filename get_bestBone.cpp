// 自适应瞄准部位
double CalculateDistanceFromCrosshair(Vector TargetPosition)
{
	uint64_t LocalPlayer = 0;
	apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
	Entity from = getEntity(LocalPlayer);

	Vector CameraPosition = from.GetCamPos();
	QAngle CurrentAngle = from.GetSwayAngles();

	if (CameraPosition.DistTo(TargetPosition) <= 0.0001f)
		return -1;

	QAngle TargetAngle = Math::CalcAngle(CameraPosition, TargetPosition);

	if (!TargetAngle.IsValid())
		return -1;

	return CurrentAngle.distanceTo(TargetAngle);
}

int GetBestBone(Entity &Target)
{
	float NearestDistance = 999;
	int NearestBone = 2;
	for (int i = 0; i < 6; i++)
	{
		double DistanceFromCrosshair = CalculateDistanceFromCrosshair(Target.getBonePositionByHitbox(i));
		if (DistanceFromCrosshair < NearestDistance)
		{
			NearestBone = i;
			NearestDistance = DistanceFromCrosshair;
		}
	}
	return NearestBone;
}
