// 定义投掷角度和视角的结构体
typedef struct
{
	float view;
	float launch;
} Pitch;

float optimal_angle(float x, float y, float v0, float g)
{
	float root = powf(v0, 4) - g * (g * powf(x, 2) + 2.0 * y * powf(v0, 2));
	if (root < 0.0f)
	{
		return NAN;
	}
	root = sqrtf(root);
	float slope = (powf(v0, 2) - root) / (g * x);
	return atanf(slope);
}

float lob_angle(float x, float y, float v0, float g)
{
	float root = powf(v0, 4) - g * (g * powf(x, 2) + 2.0 * y * powf(v0, 2));
	if (root < 0.0f)
	{
		return NAN;
	}
	root = sqrtf(root);
	float slope = (powf(v0, 2) + root) / (g * x);
	return atanf(slope);
}

float calc_angle(float dx, float dy, float v0, float g, bool lob)
{
	if (lob)
	{
		return lob_angle(dx, dy, v0, g);
	}
	else
	{
		return optimal_angle(dx, dy, v0, g);
	}
}

// 角度 to 视角
float launch2view(const Pitch *pitches, int num_pitches, float launch)
{
	if (num_pitches < 2)
	{
		return launch;
	}

	int low = 0;
	int high = num_pitches - 1;
	while (low + 1 != high)
	{
		int middle = (low + high) / 2;
		const Pitch *entry = &pitches[middle];
		if (launch < entry->launch)
		{
			high = middle;
		}
		else
		{
			low = middle;
		}
	}

	const Pitch *low_pitch = &pitches[low];
	const Pitch *high_pitch = &pitches[high];

	float fraction = (launch - low_pitch->launch) / (high_pitch->launch - low_pitch->launch);
	return low_pitch->view + fraction * (high_pitch->view - low_pitch->view);
}

// 热燃和碎片手榴弹的投掷角度和视角对应关系
Pitch GRENADE_PITCHES[49] = {
	{-1.5533, -1.3990}, // 89
	{-1.4837, -1.3267}, // 85
	{-1.3962, -1.2433}, // 80
	{-1.3092, -1.1534}, // 75
	{-1.2217, -1.0779}, // 70
	{-1.1347, -0.9783}, // 65
	{-1.0472, -0.8977}, // 60
	{-0.9602, -0.8104}, // 55
	{-0.8727, -0.7268}, // 50
	{-0.7857, -0.6375}, // 45
	{-0.6981, -0.5439}, // 40
	{-0.6112, -0.4688}, // 35
	{-0.5236, -0.3880}, // 30
	{-0.3491, -0.2050}, // 25
	{-0.3491, -0.2050}, // 20
	{-0.2615, -0.1165}, // 15
	{-0.1746, -0.0421}, // 10
	{-0.0870, 0.0644},	//  5
	{-0.0001, 0.1403},	//  0
	{0.0875, 0.2358},	// -5
	{0.1745, 0.3061},	//-10
	{0.2620, 0.3753},	//-15
	{0.3490, 0.4684},	//-20
	{0.4365, 0.5343},	//-25
	{0.5235, 0.6238},	//-30
	{0.6110, 0.6865},	//-35
	{0.6979, 0.7756},	//-40
	{0.7331, 0.7968},	//-42
	{0.7682, 0.8341},	//-44
	{0.8027, 0.8771},	//-46
	{0.8379, 0.9038},	//-48
	{0.8727, 0.9382},	//-50
	{0.9079, 0.9620},	//-52
	{0.9424, 1.0048},	//-54
	{0.9775, 1.0333},	//-56
	{1.0121, 1.0561},	//-58
	{1.0472, 1.0987},	//-60
	{1.0824, 1.1217},	//-62
	{1.1175, 1.1628},	//-64
	{1.1520, 1.1868},	//-66
	{1.1866, 1.2239},	//-68
	{1.2217, 1.2555},	//-70
	{1.2563, 1.2859},	//-72
	{1.2913, 1.3156},	//-74
	{1.3264, 1.3470},	//-76
	{1.3615, 1.3822},	//-78
	{1.3973, 1.4108},	//-80
	{1.4837, 1.4919},	//-85
	{1.5533, 1.5546}	//-89
};
Pitch ARC_PITCHES[19] = {
	{-1.5533, -1.5198},
	{-1.3967, -1.3672},
	{-1.2222, -1.1974},
	{-1.0477, -1.0260},
	{-0.8731, -0.8550},
	{-0.6986, -0.6848},
	{-0.5241, -0.5129},
	{-0.3496, -0.3416},
	{-0.1572, -0.1484},
	{0.0000, 0.0080},
	{0.1751, 0.1800},
	{0.3496, 0.3520},
	{0.5241, 0.5234},
	{0.6992, 0.6978},
	{0.8727, 0.8710},
	{1.0472, 1.0453},
	{1.2218, 1.2201},
	{1.3963, 1.3956},
	{1.5533, 1.5533}};

float to_degrees(float radians)
{
	float degrees = radians * (180.0 / M_PI);
	return degrees;
}
float to_radians(float degrees)
{
	return degrees * (M_PI / 180.0f);
}
Vector qangles(Vector v)
{
	Vector result;
	float tmp;
	float yaw;
	float pitch;

	if (v.y == 0.0f && v.x == 0.0f)
	{
		yaw = 0.0f;
		if (v.z > 0.0f)
		{
			pitch = 270.0f;
		}
		else
		{
			pitch = 90.0f;
		}
	}
	else
	{
		yaw = atan2f(v.y, v.x) * (180.0f / M_PI);
		tmp = sqrtf(v.x * v.x + v.y * v.y);
		pitch = atan2f(-v.z, tmp) * (180.0f / M_PI);
	}

	result.x = pitch;
	result.y = yaw;
	result.z = 0.0f;

	return result;
}
// 空爆雷 c++代码
QAngle skynade_angle(Entity target)
{
	// 获得武器信息
	uint64_t LocalPlayer = 0;
	apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
	uint64_t node;
	apex_mem.Read<uint64_t>(LocalPlayer + OFFSET_m_inventory, node);
	node &= 0xffff;
	uint64_t node_entity = 0;
	apex_mem.Read<uint64_t>(g_Base + OFFSET_ENTITYLIST + (node << 5), node_entity);
	float node_speed = 0;
	apex_mem.Read<float>(node_entity + OFFSET_BULLET_SPEED, node_speed);
	float node_scale = 0;
	apex_mem.Read<float>(node_entity + OFFSET_BULLET_SCALE, node_scale);
	int id = 0;
	apex_mem.Read<int>(node_entity + +OFFSET_WEAPON_NAME, id);

	if (id != 160 && id != 161)
		return QAngle(0, 0, 0);

	Entity from = getEntity(LocalPlayer);

	// 重力加速度 let g = 750.0 * weapon.projectile_scale;
	float g = 750.0 * node_scale;
	// 初始速度 let v0 = weapon.projectile_speed;
	float v0 = node_speed;

	// 计算目标点与视角原点之间的差值
	// QAngle delta0 = Math::CalcAngle(from.GetCamPos(), target.getPosition());

	Vector local_viewOffset;
	apex_mem.Read<Vector>(LocalPlayer + OFFSET_viewOffset, local_viewOffset);
	local_viewOffset += from.getPosition();
	Vector target_origin = target.getPosition();
	Vector delta = target_origin - local_viewOffset;
	Vector delta2 = delta + (delta * 20 / delta.Length());
	// printf("%.5f,%.5f || %.5f,%.5f || %.5f,%.5f\n",delta0[0],delta0[1],delta[0],delta[1],delta2[0],delta2[1]);
	float z_offset = (id == 160 ? 70.0 : 25.0);
	float dx = delta2.Magnitude2D();
	float dy = delta[2] + z_offset;

	// QAngle delta = Math::CalcAngle(from.getPosition() + local_viewOffset, target.getPosition());
	// printf("%.5f,%.5f\n",delta.x,delta.y);

	// // 计算差值在垂直方向上的距离，考虑高度偏移
	// float z_offset = 70.0;
	// float dy = delta.y + z_offset;
	// // 计算投掷角度
	float angle = calc_angle(dx, dy, v0, g, id == 160 ? true : false);
	// // 将投掷角度转换为视角角度并返回
	float view_pitch = (id == 160 ? launch2view(GRENADE_PITCHES, 49, angle) : launch2view(ARC_PITCHES, 19, angle));
	float view_yaw = to_radians(qangles(target_origin - local_viewOffset)[1]);

	if (!isnan(view_pitch))
	{
		QAngle skynade_angle(-to_degrees(view_pitch), to_degrees(view_yaw), 0);
		QAngle ViewAngles = from.GetViewAngles();

		if (abs(skynade_angle.y - ViewAngles.y) > 180)
		{
			if (skynade_angle.y > 0)
			{
				skynade_angle.y -= 360; // 将大于180度的目标角度减去360度
			}
			else
			{
				skynade_angle.y += 360; // 将小于-180度的目标角度加上360度
			}
		}

		QAngle Delta = skynade_angle - ViewAngles;
		QAngle SmoothedAngles = ViewAngles + Delta / 70;
		// printf("Delta:%.5f\n",Delta.x);
		// printf("angle:%.5f || view_pitch:%.5f\n",angle,to_degrees(view_pitch));
		// printf("View:%.5f,%.5f | node_pitch:%.5f,%.5f\n",from.GetViewAngles().x,from.GetViewAngles().y,skynade_angle.x,skynade_angle.y);
		// printf("View:%.5f,%.5f | SmoothedAngles:%.5f,%.5f\n",from.GetViewAngles().x,from.GetViewAngles().y,SmoothedAngles.x,SmoothedAngles.y);
		Math::NormalizeAngles(SmoothedAngles);
		return SmoothedAngles;
	}
	return QAngle(0, 0, 0);
}
