const {fs,path,root,chapters}=require('./api-model.cjs');
const crypto=require('crypto');
const raw=fs.readFileSync(path.join(root,'book-timers-CH11/artifacts/chapter-build-results.json'),'utf8').replace(/^\uFEFF/,'');
const rows=JSON.parse(raw);if(rows.length!==20||rows.some(r=>r.ExitCode!==0))throw Error('Expected final 20/20 Debug builds');
const all=[];
for(const[ch,name]of Object.entries(chapters)){
 const dir=path.join(root,name),src=JSON.parse(fs.readFileSync(path.join(dir,'artifacts/source-verification.json'),'utf8'));
 if(src.Failures.length)throw Error('Source verification failure '+name);
 const projectRows=rows.filter(r=>r.Project===name).map(r=>{
  const base=`build/Debug-${r.Example}/${name}-${r.Example}`;
  const outputs=['elf','hex','bin'].map(ext=>{const p=base+'.'+ext,b=fs.readFileSync(path.join(dir,p));return {Path:p,Bytes:b.length,SHA256:crypto.createHash('sha256').update(b).digest('hex')};});
  const log=`artifacts/verify-${r.Example}.log`,rawLog=fs.readFileSync(path.join(dir,log)),s=rawLog.toString(rawLog[0]===0xff&&rawLog[1]===0xfe?'utf16le':'utf8');
  if(!s.includes('Image checks passed:'))throw Error('Missing image checks '+log);
  return {...r,Date:'2026-10-31',FinalSource:true,ImageChecks:'passed',InitialStack:'0x20020000',Log:log,Outputs:outputs,Warnings:[...new Set(s.split(/\r?\n/).filter(l=>l.includes('warning:')).map(l=>l.trim()))]};
 });
 all.push(...projectRows);
 fs.writeFileSync(path.join(dir,'artifacts/build-results.json'),JSON.stringify(projectRows,null,2)+'\n');
 let md=`# ${ch} 验证记录\n\n核对日期：2026-10-31。本记录针对当前中文注释后的学习副本，不继承上游硬件验收。\n\n## 静态与源码边界\n\n- 原始参考文件SHA256核对：${src.ReferenceFiles}个全部一致，没有修改references。\n- 学习副本中${src.ByteIdentical}个原文件字节不变，${src.CommentOnlyFiles}个仅增加注释；${src.DocumentedPortExceptionFiles}个已记录的必要移植例外。\n- C词法单元对照保留字符串/字符字面量，确认除CH11 ex2补函数外未改业务token；厂商HAL/CMSIS源码全部保持原样。\n- ${src.ExternalAPIs}个外部API卡与${src.ExampleChecks}个真实接口片段已重新回查当前源码和行号；本地HAL回调定义未误计为外部调用。\n- 机器细项：source-verification.json、api-inventory.json。\n\n## 实际编译与镜像检查\n\n使用官方STM32工具包：GNU Arm GCC 14.3.1+st.2、CMake 4.3.1+st.1、Ninja 1.13.2+st.1。首次受限执行出现Windows DLL relocation，使用获准的本地构建权限重新运行；不是源码编译错误。\n\n| 配置 | 结果 | BIN字节 | 镜像检查 | 日志 |\n| --- | --- | ---: | --- | --- |\n`;
 for(const r of projectRows)md+=`| Debug-${r.Example} | 通过 | ${r.Outputs.find(o=>o.Path.endsWith('.bin')).Bytes} | 通过 | [verify-${r.Example}.log](verify-${r.Example}.log) |\n`;
 md+='\n每项均产出ELF/HEX/BIN，BIN容量不超过512KB；初始栈为0x20020000，Reset/SysTick向量为镜像Flash范围内的Thumb地址。构建脚本或日志检查通过不能推导外设运行成功。机器结果及输出SHA256见build-results.json。Release预设可用但本轮未构建，未宣称验收。\n\n使用原始链接脚本时链接器报告LOAD segment RWX；保留该警告，未擅改原始内存布局。I2C从板还有未初始化温度变量警告，其风险已写入README；该编译告警不作为正确温度输出的证据。各例原始风险、DMA IRQ缺口或测量公式问题均见README，没有在本轮悄悄改业务。\n\n## 烧录与硬件观察\n\n烧录：未执行。开发板连接：未执行。保护选项：未修改。串口、LED、波形、总线通信、功耗：均未实测。README现象表为源码预期，尤其CH19的GPIO模拟化会影响SWD、STANDBY使用PA0/WKUP1，不能把编译记录当作运行授权或先修要求。\n';
 fs.writeFileSync(path.join(dir,'artifacts/verification.md'),md);
 console.log(name,projectRows.length,'Debug builds recorded');
}
fs.writeFileSync(path.join(root,'book-timers-CH11/artifacts/chapter-build-results.json'),JSON.stringify(all,null,2)+'\n');
