const {fs,path,root,chapters,mask}=require('./api-model.cjs');
const {describe,fields}=require('./study-details.cjs');
function apiComment(name){try{return describe(name);}catch{return null;}}
const functionPurpose={main:'本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程',Nucleo_BSP_Init:'公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA',SystemClock_Config:'建立本章 PLL 与总线分频；频率必须以本函数参数推导，不能跨章节照搬',MX_GPIO_Init:'使能端口时钟并配置板载按键/LED；复用引脚由对应外设初始化接管',MX_GPIO_Deinit:'将所有 PA/PB/PC 引脚设为模拟并关闭端口时钟；含 SWD 引脚，调试连接可能丢失',MX_DMA_Init:'使能 DMA 控制器并配置本例所需的完成中断通道',MX_USART2_UART_Init:'设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察',MX_ADC1_Init:'配置内部温度通道的 ADC1 规则转换及本例 DMA 关联',MX_DAC_Init:'配置 DAC 通道引脚、触发源与可选的 DMA 正弦表搬运',MX_I2C1_Init:'配置 PB8/PB9 为 I2C1 开漏复用，设置 100 kHz 总线与寻址参数',MX_TIM1_Init:'建立 TIM1 的分频、周期与本例的触发/比较/更新请求',MX_TIM2_Init:'建立 TIM2 的计数参数；PWM与ADC触发子例用途不同，以本文件为准',MX_TIM3_Init:'建立 TIM3 的捕获/比较/编码器等模式和通道连接',MX_TIM6_Init:'建立 TIM6 周期更新并输出 TRGO，作为 DAC 取样节拍',computePulse:'按 PCLK1 与 PSC 计算比较间隔；注意定时器倍频与 TOGGLE完整周期因素',Read_From_24LCxx:'先发送两字节存储地址，再阻塞接收 EEPROM 数据并返回 HAL 状态',Write_To_24LCxx:'拼接两字节存储地址与载荷，发送并轮询 EEPROM 写周期完成；原例缺判空/失败释放',SleepMode:'撤销不必要资源、暂停时基后请求 SLEEP，唤醒后恢复引脚和串口',StopMode:'撤销资源并进入 STOP；唤醒后重配 PLL 与串口，再恢复时基',StandbyMode:'重新使能 PA0 WKUP1 后进入 STANDBY，唤醒使芯片复位而非返回调用点',Error_Handler:'错误分支入口；原例若关闭中断并死循环，则只能调试定位或复位恢复',assert_failed:'USE_FULL_ASSERT 条件下的参数断言模板，原例仅保留空实现',HAL_TIM_PeriodElapsedCallback:'由 HAL_TIM_IRQHandler 在更新事件后调用，按 Instance 判断并翻转 LD2',HAL_TIM_OC_DelayElapsedCallback:'比较匹配后的 HAL 回调：推进对应 CCR，安排下一次翻转时刻',HAL_TIM_IC_CaptureCallback:'输入捕获 DMA 完成的 HAL 回调：置位 captureDone，主循环再处理数组',HAL_ADC_ConvCpltCallback:'ADC DMA 缓冲区完成回调：只置 convCompleted 标志，格式化放在主循环',HAL_I2C_AddrCallback:'从机地址匹配回调：保存传输方向并通知主循环准备寄存器响应',HAL_GPIO_EXTI_Callback:'按键 EXTI 回调：原例忙等 PC13 释放，需注意中断上下文阻塞',SysTick_Handler:'Cortex-M 毫秒中断入口：递增 HAL tick，支撑延时和超时判断'};
function funText(name){return functionPurpose[name]||(name.endsWith('_IRQHandler')?'硬件中断入口：将当前事件交给本外设的 HAL 分派器':name.includes('MspDeInit')?'HAL 撤销外设时调用的资源释放回调；主业务未必走到此模板':name.includes('MspInit')?'HAL 初始化过程调用的资源回调：使能时钟并配置对应引脚/DMA/IRQ':name.endsWith('_Handler')?'处理器异常入口；保留原例的等待/默认处理方式':null);}
function meaning(line,prev){
 const t=line.trim();if(!t||/^[{}]+;?$/.test(t))return null;
 if(t.startsWith('#include')){const f=t.match(/[<"]([^>"]+)/)?.[1];return f?.includes('stm32')?'引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明':f==='main.h'?'引入本工程板级引脚别名和错误处理声明':f==='nucleo_hal_bsp.h'?'引入本工程公共板级初始化入口':`引入 ${f}，供本文件的字符串/格式化/数学或运行库处理使用`;}
 if(t.startsWith('#ifndef')||t.startsWith('#define')||t.startsWith('#ifdef')||t.startsWith('#if ')||t.startsWith('#else')||t.startsWith('#endif')){
  if(t.includes('SLAVE_BOARD'))return '编译选择：定义 SLAVE_BOARD 时保留从板温度/寄存器逻辑，否则构建主板读取逻辑';
  if(t.includes('USE_FULL_ASSERT'))return '编译选择：仅启用完整参数断言时保留该回调模板';
  if(/^#define\s+\w+_Pin\s/.test(t))return '板级引脚位掩码：与对应 GPIO_Port 一起传入 HAL GPIO 接口';
  if(/^#define\s+\w+_GPIO_Port\s/.test(t))return '板级端口别名：与对应 Pin 配对定位真实 GPIO';
  if(/^#define\s+HAL_\w+_MODULE_ENABLED/.test(t))return '启用此 HAL 功能模块，使 hal.h 引入其头文件及模块实现';
  if(/^#define\s+WHO_AM_I_VALUE\b/.test(t))return '定义身份寄存器的固定响应字节0xBC，主板读取WHO_AM_I寄存器时用它辨识从板';
  if(/^#define\s+(?:HSE|HSI|LSE|LSI)_VALUE\b/.test(t))return '提供振荡源标称频率常量；须与本章硬件和时钟选择一致';
  if(t.startsWith('#endif'))return '结束本段条件编译分支或头文件保护范围';
  if(t.startsWith('#else'))return '选择上方条件不成立时的另一编译分支';
  if(t.includes('assert_param'))return '声明参数断言宏：启用完整断言时报告错误，否则保留为空操作';
  if(t.startsWith('#define'))return '定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致';
  return '条件编译边界：仅在对应宏条件成立时保留下方声明或配置';
 }
 const def=t.match(/^(?:static\s+)?(?:void|int|uint\w+|HAL_StatusTypeDef)\s+(\w+)\s*\(/);
 if(def){const txt=funText(def[1]);if(txt)return txt+(t.endsWith(';')?'（此处为声明）':'');}
 const a=t.match(/(?:\.|->)(\w+)\s*=(?!=)/);if(a){const key=a[1].replace(/^IC[12]/,'IC');if(fields[key])return fields[key];}
 const calls=[...t.matchAll(/\b([A-Za-z_]\w*)\s*\(/g)].map(x=>x[1]);
 for(const n of calls){const d=apiComment(n)||funText(n);if(d)return d;}
 if(/^while\s*\(\s*(?:1|!convCompleted|!transferRequested)/.test(t))return t.includes('convCompleted')?'等待 DMA 完成回调置位后再读取缓冲区；本例未设置超时':t.includes('transferRequested')?'无 I2C 请求时继续采温度，地址匹配回调置位后跳出':'进入持续业务或空转等待；是否更新输出取决于循环体';
 if(/^while/.test(t))return '按当前电平/计数/状态保持等待；退出条件必须由硬件、回调或循环体更新';
 if(/^if/.test(t))return t.includes('capture')?'区分捕获先后与回绕，以计算两个边沿的计数差':t.includes('HAL_OK')?'核对 HAL 返回值；错误分支按原例处理，未统一添加恢复机制':'根据此处状态选择后续处理分支，保持原例条件不变';
 if(/^else/.test(t))return '处理上一条件不成立的分支，与上方正常/回绕条件配对';
 if(/^for/.test(t))return t.includes('SAMPLES')||t.includes('200')?'逐点生成一个周期的查找表，供 DMA 后续按顺序输出':'按序处理本例的采样组或表项，边界取自实际配置';
 if(/^(case|default)/.test(t))return '按主机请求的寄存器索引选择身份、温度整数/小数或默认返回值';
 if(/^switch/.test(t))return '分派从 I2C 接收到的寄存器索引，准备单字节响应';
 if(/^break/.test(t))return '结束当前寄存器分支，避免继续执行下一项';
 if(/^return/.test(t))return '把当前计算结果或 HAL 状态交给调用者；不隐含硬件验收结论';
 if(t==='extern "C" {')return '让C++调用者按C链接约定引用本头文件函数';
 if(/^extern/.test(t))return '引用其他源文件定义的共享符号，不在此处再次分配存储';
 if(/\b(?:ADC|DAC|TIM|UART|I2C|DMA)_HandleTypeDef\b/.test(t))return '保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用';
 if(/(?:InitTypeDef|ConfTypeDef|ConfigTypeDef)/.test(t))return '为本次硬件初始化准备配置结构，随后逐字段设置有效参数';
 if(t.includes('rawValue')&&t.includes('/ 4095'))return '把 12 位采样码按假定 3.3 V 参考换算为毫伏；真实参考电压误差会带入温度';
 if(t.includes('760.0')&&t.includes('2.5'))return '用原例的 25℃/760mV/2.5mV每℃公式估算芯片结温，不能当作校准环境温度';
 if(t.includes('captureDone =')||t.includes('convCompleted =')||t.includes('transferRequested ='))return '在主循环与中断回调之间传递完成/请求标志，volatile 防止读取被优化缓存';
 if(t.includes('speed ='))return '保留原例速度计算；该除以60的公式与RPM量纲不符，README说明未作为可靠测速';
 if(t.includes('frequency ='))return '按定时器计数频率和捕获差值换算频率；分频、回绕和零差值需另核对';
 if(t.includes('lastConversion'))return '保存/声明最近采温时间；原从机变量未初始化，首次时间判断存在风险';
 if(t.includes('i2cBuf['))return '准备 I2C 寄存器索引或单字节响应内容，与主从顺序帧配对';
 if(t.includes('data[0]')||t.includes('addr[0]'))return '取出 EEPROM 存储地址高字节，先于低字节发送';
 if(t.includes('data[1]')||t.includes('addr[1]'))return '取出 EEPROM 存储地址低字节，与高字节组成16位存储地址';
 if(/^\w+\[[^\]]+\]\s*=/.test(t))return '将当前计算样本保存到数组，后续通过 DMA 或串口消费';
 if(/^(?:volatile\s+)?(?:const\s+)?(?:uint\w+|int\w*|float|double|char|HAL_StatusTypeDef)\s/.test(t))return t.includes('[')?'分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界':t.includes('volatile')?'声明主循环与中断共享的状态，保证每次访问读取实际存储':'声明本例的计数、状态或计算中间值，初始值以当前语句为准';
 if(/\b(?:diff|cnt[12]|tick|dir|pulse|tim1_ch\w+|t_int|t_frac|ftemp|temp|dutyCycle|value|angle)\s*(?:[+\-*/]?=|\+\+|--)/.test(t))return '更新本例计数差、相位、温度或幅值中间量，供后续输出使用';
 if(t==='extern "C" {')return '让 C++ 调用者按 C 链接约定引用本头文件的函数';
 if(t.startsWith('assert_param'))return '按配置检查参数；未启用完整断言时宏不执行报告';
 if(t.startsWith('((void)'))return '显式消费未使用参数，保留模板签名以匹配调用约定';
 return null;
}
for(const [ch,name]of Object.entries(chapters)){
 const dir=path.join(root,name);let count=0;
 const files=fs.readdirSync(path.join(dir,'Core/Src')).filter(n=>/^main.*\.c$/.test(n)||['nucleo_hal_bsp.c','stm32f4xx_hal_msp.c','stm32f4xx_it.c'].includes(n)).map(n=>'Core/Src/'+n).concat(fs.readdirSync(path.join(dir,'Core/Inc')).filter(n=>n.endsWith('.h')).map(n=>'Core/Inc/'+n));
 for(const file of files){const p=path.join(dir,file),s=fs.readFileSync(p,'utf8');if(s.includes('// 学习：'))continue;
  const raw=s.split(/\r?\n/),clean=mask(s).split(/\r?\n/);const out=[];
  raw.forEach((line,i)=>{const trimmed=line.trim();if(trimmed&&!/^\s*(?:\/\*|\*|\/\/)/.test(line)&&clean[i]?.trim()){
   const comment=meaning(line,raw[i-1]);if(comment){out.push(line.match(/^\s*/)[0]+'// 学习：'+comment+'。');count++;}
  }out.push(line);});
  const updated=out.join('\n');if(mask(s).replace(/\s/g,'')!==mask(updated).replace(/\s/g,''))throw Error('Token changed '+p);
  fs.writeFileSync(p,updated,'utf8');
 }
 console.log(ch,'Chinese teaching comments',count);
}
