    # READEME

    // 1. 参数校验
    // 3. 测试流程
    // 2. parser

    // 未知指令？
    // 参数过少？
    // 参数类型? 整型， 字符串
    //

    // PC <--> tester <--> master
    // Speaker
    // Mic
    // PG  信号生成器
    // MNT 监视器

    // 时间单位统一 : ms
    // 频率单位统一 : Hz

    // PC 控制端指令
    // sleep 500    -- PC暂停测试流程，延时等待500ms
    // record 500   -- PC左+右两麦克风开始录制，录制时长500ms,(注意：录制过程中测试流程暂停，不进行下一步操作)

    // set_order L R  -- 告知 左右扬声器播放顺序 先左后右(默认)
    // set_order R L  -- 告知 左右扬声器播放顺序 先右后左

    //                  时刻指针 时刻±范围  频率  频率误差±
    // get_audio_info 1  1500    200     1000   200    -- 立即开始获取音频信息 ，读取工作目录下L1.wav+R1.wav文件 1500ms±200 时段提取音频信息, 结果存放在内存中 ,包含强度level 频率pitch
    // get_audio_info 2  2000    300     2500   300    -- 立即开始获取音频信息 ，读取工作目录下L2.wav+R2.wav文件 2000ms±300 时段提取音频信息, 结果存放在内存中 ,包含强度level 频率pitch

    // autotest_start // 默认第一条, 保留，可不写
    // autotest_end   // 测试流程结束
    // 根据内存中已存储的数据在界面给出结果，并发送pass|fail给AutoLine
    // 测试规则固定。 根据 1. 指定的扬声器播放顺序 2. 强度信息（4个） 3. 频率信息（4个）， 判断扬声器状态 , 两扬声器正常 pass，否则fail
    // (备注，务必保证该指令前已经获取了L1.wav+R1.wav L2.wav+R2.wav 的信息)

    // sendcmd2pg RUN PATTERN 103;  -- 指令由用户自定义
    // sendcmd2mnt RUN PATTERN 103;

# 频率侦测
* 手动|自动 模式设定. 手动不需要侦听频率
* 设定侦听超时. XXX ms
* 设定侦听频率满足范围. 1000(±100)Hz

set_intercept_timeout 10000
set_intercept_freq 1000 200
