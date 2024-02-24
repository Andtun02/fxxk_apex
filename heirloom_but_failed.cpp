if(vars->misc.heirloom){ /* 传家宝 */
            // 获得手持模型实体
            auto view_model_handle = apex_mem.Read<uint64_t>(LocalPlayer + offsets.cplayer_viewmodels); //m_hViewModels
            view_model_handle &= 0xFFFF;
            auto view_model_ptr = apex_mem.Read<uint64_t>(g_Base + offsets.entitylist + (view_model_handle << 5));
            if (!view_model_ptr) continue;
            // 获得模型名称
            char modelName[200] = {0};
            auto name_ptr = apex_mem.Read<uint64_t>(view_model_ptr + offsets.centity_modelname);
            apex_mem.ReadArray<char>(name_ptr, modelName, 200);
            std::string model_name_str= std::string(modelName);
            // 获得动画序列
            int cur_sequence,modelAniIndex;
            // 写入模型参数
            if (model_name_str.find("emptyhand") != std::string::npos) { // 116挥拳 是否空手
                apex_mem.Write<const char*>(name_ptr, "mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl");
                apex_mem.Write(view_model_ptr + m_currentFrame_modelIndex,3605);
            }else if(model_name_str.find("ptpov_baton_lifeline")!= std::string::npos){ // 是否为传家宝
                // 获得动画序列
                cur_sequence = apex_mem.Read<int>(view_model_ptr + m_currentFrameBaseAnimating_animSequence);
                modelAniIndex = apex_mem.Read<int>(view_model_ptr + m_currentFrameBaseAnimating_animModelIndex);
                if(cur_sequence == 0 && modelAniIndex == 3605){
                    apex_mem.Write(view_model_ptr + m_currentFrameBaseAnimating_animSequence,82);
                }
            }
 
            // 获得模型下标
            // auto modelIndex = apex_mem.Read<short>(view_model_ptr + m_currentFrame_modelIndex);
            // melog(debug,"[%d][%d][%d] Ptr:%lx,Model:%s",modelIndex,modelAniIndex,cur_sequence,view_model_ptr,modelName);
        }
			/* 总结：
            *  方法一、需要找到 [当前动作] 对应的 [动画序列]
            *  方法二、设置模型与ID后 [动画序列] = 0，找到导致的原因
            * */

/** 
The garbage code generated during the process, if it is helpful to you
The garbage code generated during the process, if it is helpful to you
The garbage code generated during the process, if it is helpful to you
The garbage code generated during the process, if it is helpful to you
The garbage code generated during the process, if it is helpful to you

 { /* 传家宝 */
            // 获得手持模型实体
            auto view_model_handle = apex_mem.Read<uint64_t>(LocalPlayer + offsets.cplayer_viewmodels); //m_hViewModels
            view_model_handle &= 0xFFFF;
            auto view_model_ptr = apex_mem.Read<uint64_t>(g_Base + offsets.entitylist + (view_model_handle << 5));
            if (!view_model_ptr) continue;
            // 获得模型下标
            auto modelIndex = apex_mem.Read<short>(view_model_ptr + m_currentFrame_modelIndex);
            char modelName[200] = {0};
            auto name_ptr = apex_mem.Read<uint64_t>(view_model_ptr + offsets.centity_modelname);
            apex_mem.ReadArray<char>(name_ptr, modelName, 200);

            /**
             *  重点：根据拳头的 [当前动作] 获得对应传家宝的 [动画序列] 然后写入
             *  ========
             *  根据起源引擎的代码来看有一个方法，获得szactivitynameindex也是可行的
             *
             * 	int					szactivitynameindex;
             * 	inline char * const pszActivityName( void ) const { return ((char *)this) + szactivitynameindex; }
             *
             */
            if (std::string(modelName).find("emptyhand")) { // 是否空手
                // Get 获得动画序列 [旧序列=> cur_sequence]
                const auto cur_sequence = apex_mem.Read<int>(view_model_ptr + m_currentFrameBaseAnimating_animSequence);
                char m_predictedAnimEventData[200];
                apex_mem.ReadArray<char>(view_model_ptr + 0x0a68,m_predictedAnimEventData,200);
                //m_predictedAnimEventData 0x0a68

                // Get 根据模型=>序列 获得 [动画名称 => 哈希值]
                // 动画哈希值 = [空手]viewmodel_entity.get([拳头]cur_sequence)
                /*伪代码：
                 * char pszActivityName[200];
                 * apex_mem.ReadArray<char>(view_model_ptr + offset.?,200);
                 */


                // 写入模型数据
                //apex_mem.Write<const char*>(name_ptr, "mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl");
//                apex_mem.WriteArray<char>(name_ptr, "mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl",strlen("mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl"));
//                apex_mem.Write(view_model_ptr + m_currentFrame_modelIndex,3605);
//                apex_mem.Write(view_model_ptr + m_currentFrameBaseAnimating_animModelIndex,3605);

                // Set 根据 [动画哈希值] 设置 [新序列=> new_sequence]
                // new_sequence = [传家宝]viewmodel_entity.get([拳头]动画哈希值)
                /*伪代码：
                 * int new_sequence = apex_mem.Read<int>(view_model_ptr + pszActivityName + 动作);
                 * if (new_sequence) apex_mem.Write(view_model_ptr + m_currentFrameBaseAnimating_animSequence,new_sequence);
                 */

                // 笨蛋方案
//                if ((cur_sequence == 0 ||cur_sequence == 28)  && modelIndex == 3605 && std::string(modelName).find("ptpov_baton_lifeline") != std::string::npos) {
//                    apex_mem.Write<int>(view_model_ptr + m_currentFrameBaseAnimating_animSequence, 82);
//                }

                printf("cur_sequence:%d,cur_modeindex:%d,%s,modelName:%s\n",cur_sequence,modelIndex,m_predictedAnimEventData,modelName);
//                cur_sequence:3,cur_modeindex:185,modelName:mdl/weapons/empty_handed/ptpov_emptyhand.rmdl      未写、切换出来默认状态
//                cur_sequence:0,cur_modeindex:3605,modelName:mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl   写入、切换出来默认状态
//                cur_sequence:82,cur_modeindex:3605,modelName:mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl  写入、切换出来默认状态

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

//
//
//
//
//
//                    apex_mem.WriteArray<char>(name_ptr, "mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl",strlen("mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl"));
//                    apex_mem.ReadArray<char>( name_ptr, modelName, 200);
//                    modelIndex =  apex_mem.Read<short>(view_model_ptr + m_currentFrame_modelIndex);
//                    if(strcmp(modelName,set_model_name)) printf("modelIndex=> %d\n",modelIndex);
                // 获取视图模型实体的当前帧动画序列号
//                const auto cur_sequence = apex_mem.Read<int>(view_model_ptr + m_currentFrameBaseAnimating_animSequence);
//                const auto cur_modeindex = apex_mem.Read<int>(view_model_ptr + m_currentFrameBaseAnimating_animModelIndex);
                // 获得动画状态
                // 写入命脉传家宝
//                int u_modelIndex = 3605;
//                apex_mem.Write(view_model_ptr + m_currentFrame_modelIndex,u_modelIndex);
//                apex_mem.Write<int>(view_model_ptr + m_currentFrameBaseAnimating_animModelIndex,u_modelIndex);
//                apex_mem.WriteArray<char>(name_ptr, "mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl",strlen("mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl"));
                // 写入动画状态
                /*拳头静止28|拳头模型185*/
//                    int seq = 82;/*电棒静止为82*/
//                    apex_mem.Write<int>(view_model_ptr + m_currentFrameBaseAnimating_animSequence,seq);
                //debug
//                printf("ptr:%lx,hanndle:%lx,vmp:%lx\n",model1,model2,view_model_ptr); //ptr:16da6cb0e00,hanndle:0,vmp:16ed3ad9d20
//                if(cur_sequence!=0) printf("cur_sequence:%d,cur_modeindex:%d,modelIndex:%d\n",cur_sequence,cur_modeindex,modelIndex);
//                printf("[%d][%s]\n",index,modelName);
//            }
            // 写入模型下标
//            apex_mem.Write(view_model_ptr + m_currentFrame_modelIndex,3605);
//            short index =  apex_mem.Read<short>(view_model_ptr + m_currentFrame_modelIndex);
//            // 获得模型名称
//            char modelName[200] = {0};
//            uint64_t name_ptr = apex_mem.Read<uint64_t>(view_model_ptr + offsets.centity_modelname);
            // 写入模型名称
//            apex_mem.Write<const char*>(name_ptr,"mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl");
//            apex_mem.WriteArray<char>(name_ptr, "mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl",strlen("mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl"));
//            apex_mem.ReadArray<char>(name_ptr, modelName, 200);
//            printf("[%d][%s]\n",index,modelName);
//          [185][mdl/weapons/empty_handed/ptpov_emptyhand.rmdl] 空手
//          [3605][mdl/weapons/drumstick/ptpov_baton_lifeline.rmdl] 命脉 [静止82,跑步21=>跑步中60=>停下67=>82,击打41=>48=>82,下蹲58=>下蹲中79=>59=>82]
//          [912][mdl/weapons/kunai/ptpov_kunai_wraith.rmdl] 恶灵
//          heirloom_wraith = 65,
//          heirloom_valkyrie = 93,
        }

**/



/*
It is indeed difficult to implement sequences externally, especially since I am still a beginner. You must go to IDA to implement the key code externally. 
I gave up and don't ask me any more related questions. I hope the following information is helpful to you
*/

// [new] https://www.unknowncheats.me/forum/3727019-post12155.html
// [old] https://www.unknowncheats.me/forum/apex-legends/488411-heirloom-animation-fix.html
// ===============
// [IDA Decompilation]
// => [pSeqdesc] pSeqdesc 48 8B 41 ? 48 85 C0 75 ? 4C 8B 49 ? 45 0F B7 41 ?
__int64 __fastcall sub_7FF65B543BF0(__int64 model, unsigned __int16 sequence)
{
  __int64 v2; // rax
  __int64 v3; // r9

  v2 = *(_QWORD *)(model + 16);
  if ( v2 )
    return *(_QWORD *)(*(_QWORD *)(v2 + 16) + 16i64 * (__int16)sequence + 8);
  v3 = *(_QWORD *)(model + 8);
  if ( sequence < *(_WORD *)(v3 + 122) )
    LOWORD(v2) = sequence;
  return v3
       + ((unsigned __int16)(*(_WORD *)(v3 + 124) & 0xFFFE) << (4 * (*(_WORD *)(v3 + 124) & 1)))
       + 112i64 * (__int16)v2;
}
// => [get_model] 40 53 48 83 EC ? 48 83 B9 ? ? ? ? ? 48 8B D9 75 ? 48 8B 41 ?
__int64 __fastcall sub_7FF65B873C40(__int64 view_model_ptr)
{
  if ( *(_QWORD *)(view_model_ptr + 0xFD0) )
    return *(_QWORD *)(view_model_ptr + 0xFD0);
  if ( (*(__int64 (__fastcall **)(__int64))(*(_QWORD *)(view_model_ptr + 16) + 96i64))(view_model_ptr + 16) )
    sub_7FF65BA9CB30(view_model_ptr);
  return *(_QWORD *)(view_model_ptr + 0xFD0);
}
























