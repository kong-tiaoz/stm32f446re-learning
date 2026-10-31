// Reproducible scaffold generator; never writes the reference directory.
const fs = require('fs'), path = require('path'), crypto = require('crypto');
const root = path.resolve(__dirname, '../..');
const chapters = {CH11:'book-timers-CH11',CH12:'book-adc-CH12',CH13:'book-dac-CH13',CH14:'book-i2c-CH14',CH19:'book-low-power-CH19'};
const write = (p,s) => fs.writeFileSync(p,s,'utf8');
const walk = p => fs.readdirSync(p,{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(path.join(p,e.name)):[path.join(p,e.name)]);
for(const [ch,name] of Object.entries(chapters)) {
 const dest=path.join(root,name), src=path.join(root,'references/stm32f446re-learning/Nucleo-F446RE',ch);
 const files=walk(src).map(p=>({Path:path.relative(src,p).replaceAll('\\','/'),SHA256:crypto.createHash('sha256').update(fs.readFileSync(p)).digest('hex')}));
 write(path.join(dest,'UPSTREAM.json'),JSON.stringify({Repository:'https://github.com/cnoviello/mastering-stm32-2nd',Commit:'2183a2c5fe25fd9229abd21dab4f23658036dd3f',Directory:`Nucleo-F446RE/${ch}`,CopiedDate:'2026-10-31',Files:files},null,2)+'\n');
 const xml=fs.readFileSync(path.join(src,'.cproject'),'utf8');
 const configs=[...xml.matchAll(/<configuration\b[^>]*\bname="([^"]+)"[\s\S]*?<sourceEntries>([\s\S]*?)<\/sourceEntries>/g)].map(m=>{
  const exc=(m[2].match(/excluding="([^"]*)"[^>]*name="Core"/)||[])[1]||'';
  const mains=fs.readdirSync(path.join(src,'Core/Src')).filter(n=>/^main.*\.c$/.test(n)&&!exc.split('|').includes('Src/'+n));
  if(mains.length!==1)throw Error(`${ch} ${m[1]} has ${mains}`);
  return {name:m[1],main:mains[0]};
 });
 if(!configs.length)throw Error(`No configurations ${ch}`);
 const variants=configs.map(c=>({name:'ex'+c.main.match(/ex(\d+)/)[1],main:c.main,origin:c.name}));
 if(ch==='CH14') variants.push({name:'ex2-slave',main:'main-ex2.c',origin:'CH14-EX2 + SLAVE_BOARD',slave:true});
 write(path.join(dest,'examples.json'),JSON.stringify(variants,null,2)+'\n');
 const sourceEntries=variants.map(v=>`if(BOOK_EXAMPLE STREQUAL "${v.name}")\n  set(EXAMPLE_SOURCE "Core/Src/${v.main}")${v.slave?'\n  set(EXAMPLE_DEFINITIONS SLAVE_BOARD)':''}\nendif()`).join('\n');
 const baseline=fs.readFileSync(path.join(root,'book-debugging/CMakeLists.txt'),'utf8');
 let cm=baseline.replace('project(book-debugging LANGUAGES C ASM)',`project(${name} LANGUAGES C ASM)\nset(BOOK_EXAMPLE "ex1" CACHE STRING "Selected chapter example")\nset_property(CACHE BOOK_EXAMPLE PROPERTY STRINGS ${variants.map(v=>v.name).join(' ')})\n${sourceEntries}\nif(NOT EXAMPLE_SOURCE)\n  message(FATAL_ERROR "Unknown BOOK_EXAMPLE: \${BOOK_EXAMPLE}")\nendif()`);
 cm=cm.replace('set(FIRMWARE ${PROJECT_NAME})','set(FIRMWARE ${PROJECT_NAME}-${BOOK_EXAMPLE})');
 cm=cm.replace(/    Core\/Src\/main-ex1\.c\n    Core\/Src\/retarget\.c\n    port\/uart_seek\.c/,`    \${EXAMPLE_SOURCE}${ch!=='CH12'?'\n    Core/Src/nucleo_hal_bsp.c':''}`);
 cm=cm.replace('PRIVATE USE_HAL_DRIVER STM32F446xx','PRIVATE USE_HAL_DRIVER STM32F446xx ${EXAMPLE_DEFINITIONS}');
 cm=cm.replace('-Wall;-Wextra;-Wno-unused-parameter','-Wall;-Wextra;-Wno-unused-parameter;-fcommon');
 cm=cm.replace('-Wl,--print-memory-usage -Wl,-z,noexecstack)','-Wl,--print-memory-usage -Wl,-z,noexecstack -Wl,-u,_printf_float)');
 write(path.join(dest,'CMakeLists.txt'),cm);
 const cp=[{name:'base',hidden:true,generator:'Ninja',toolchainFile:'${sourceDir}/cmake/arm-none-eabi-gcc.cmake',cacheVariables:{CMAKE_BUILD_TYPE:'Debug'}}];
 const bp=[];
 for(const v of variants)for(const mode of ['Debug','Release']){
  const pn=`${mode}-${v.name}`;
  cp.push({name:pn,inherits:'base',displayName:`${ch} ${v.name} ${mode}`,binaryDir:'${sourceDir}/build/'+pn,cacheVariables:{CMAKE_BUILD_TYPE:mode,BOOK_EXAMPLE:v.name}});
  bp.push({name:pn,configurePreset:pn});
 }
 write(path.join(dest,'CMakePresets.json'),JSON.stringify({version:3,cmakeMinimumRequired:{major:3,minor:22,patch:0},configurePresets:cp,buildPresets:bp},null,2)+'\n');
 let build=fs.readFileSync(path.join(root,'book-debugging/scripts/build.ps1'),'utf8');
 build=build.replace("param([ValidateSet('Debug','Release')][string]$Configuration = 'Debug')",`param([ValidateSet('Debug','Release')][string]$Configuration = 'Debug', [ValidateSet(${variants.map(v=>`'${v.name}'`).join(',')})][string]$Example = 'ex1')`);
 build=build.replace('Push-Location $script:ProjectRoot',"$preset = $Configuration + '-' + $Example\n$logDirectory = Join-Path $script:ProjectRoot 'artifacts'\nNew-Item -ItemType Directory -Path $logDirectory -Force | Out-Null\nStart-Transcript -Path (Join-Path $logDirectory ($preset + '.log')) -Force | Out-Null\nPush-Location $script:ProjectRoot");
 build=build.replaceAll('--preset $Configuration','--preset $preset');
 build=build.replace('} finally { Pop-Location }','} finally { Pop-Location; Stop-Transcript | Out-Null }');
 build=build.replace("('build\\' + $Configuration)","('build\\' + $preset)");
 build=build.replace("$binPath = Join-Path $buildDirectory 'book-debugging.bin'",`$firmware = '${name}-' + $Example\n$binPath = Join-Path $buildDirectory ($firmware + '.bin')`);
 build=build.replaceAll("'book-debugging.elf'","($firmware + '.elf')").replaceAll("'book-debugging.hex'","($firmware + '.hex')");
 write(path.join(dest,'scripts/build.ps1'),build);
 const tasks=variants.map((v,i)=>({label:`Build Debug ${v.name}`,type:'process',command:'powershell.exe',args:['-NoProfile','-ExecutionPolicy','RemoteSigned','-File','${workspaceFolder}/scripts/build.ps1','-Example',v.name],options:{cwd:'${workspaceFolder}'},group:{kind:'build',isDefault:i===0},problemMatcher:'$gcc'}));
 tasks.push({label:'Build Release (choose example)',type:'process',command:'powershell.exe',args:['-NoProfile','-ExecutionPolicy','RemoteSigned','-File','${workspaceFolder}/scripts/build.ps1','-Configuration','Release','-Example','${input:example}'],options:{cwd:'${workspaceFolder}'},group:'build',problemMatcher:'$gcc'});
 write(path.join(dest,'.vscode/tasks.json'),JSON.stringify({version:'2.0.0',tasks,inputs:[{id:'example',type:'pickString',description:'选择子例',options:variants.map(v=>v.name),default:'ex1'}]},null,2)+'\n');
 write(path.join(dest,'.vscode/settings.json'),JSON.stringify({'cmake.useCMakePresets':'always','cmake.configureOnOpen':false,'C_Cpp.default.compileCommands':'${workspaceFolder}/build/Debug-ex1/compile_commands.json'},null,2)+'\n');
 write(path.join(dest,'NOTICE.md'),`# 来源与移植边界\n\n源代码复制自本仓库 references/stm32f446re-learning/Nucleo-F446RE/${ch}，对应上游提交 2183a2c5fe25fd9229abd21dab4f23658036dd3f。原版权声明保留；原始文件逐项 SHA256 见 UPSTREAM.json。复制上游配置并不宣称 CubeIDE 配置已由本轮重新验证。\n\n新增 CMake、VS Code 任务、构建脚本、中文源码注释与学习说明。HAL/CMSIS 厂商源码保持原样。随附 licenses 保存已有工程中的 ST BSD 与 Apache 许可文本；具体文件以其原声明为准。\n\n使用 -fcommon 兼容历史工程在 main 与 BSP 中重复的暂定全局变量定义；显式链接数学库并启用浮点 printf。只做编译，不提供自动烧录任务。业务公式、参数和错误处理保留原例，已知风险见 README。\n`);
 console.log(ch,variants.map(v=>v.name).join(','));
}
