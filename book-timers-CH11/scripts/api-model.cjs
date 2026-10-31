const fs=require('fs'),path=require('path');
const root=path.resolve(__dirname,'../..');
const chapters={CH11:'book-timers-CH11',CH12:'book-adc-CH12',CH13:'book-dac-CH13',CH14:'book-i2c-CH14',CH19:'book-low-power-CH19'};
function mask(s){return s.replace(/\/\*[\s\S]*?\*\/|\/\/[^\n\r]*|"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'/g,m=>m.replace(/[^\n\r]/g,' '));}
function analyze(dir){
 const files=fs.readdirSync(path.join(dir,'Core/Src')).filter(n=>n.endsWith('.c')&&n!=='system_stm32f4xx.c');
 const defs=new Set(),refs=[];
 for(const f of files){const rel='Core/Src/'+f,s=fs.readFileSync(path.join(dir,rel),'utf8'),m=mask(s);
  const re=/\b([A-Za-z_]\w*)\s*\(/g; let hit;
  while((hit=re.exec(m))){const name=hit[1];if(['if','for','while','switch','sizeof','return','_Static_assert'].includes(name))continue;
   let p=m.indexOf('(',hit.index),lev=1,q=p+1;for(;q<m.length&&lev;q++){if(m[q]==='(')lev++;if(m[q]===')')lev--;}
   let tail=q;while(/\s/.test(m[tail]||'')&&tail<m.length)tail++;
   const lineStart=m.lastIndexOf('\n',hit.index)+1,prefix=m.slice(lineStart,hit.index).trim();
   if(m[tail]==='{'){defs.add(name);continue;}
   if(/^extern\s/.test(prefix)||['__attribute__','defined','endif'].includes(name))continue;
   if(m[tail]===';'&&/^(?:extern\s+|static\s+)?(?:void|int|float|char|uint\w+|HAL_StatusTypeDef|caddr_t|off_t|pid_t|clock_t)\s+\**$/.test(prefix))continue;
   refs.push({name,file:rel,line:s.slice(0,hit.index).split('\n').length,start:hit.index,end:q,call:s.slice(hit.index,q),source:s});
  }
 }
 return {files,defs,refs:refs.filter(r=>!defs.has(r.name))};
}
module.exports={fs,path,root,chapters,mask,analyze};
if(require.main===module)for(const [ch,name]of Object.entries(chapters)){const a=analyze(path.join(root,name));console.log(ch,[...new Set(a.refs.map(r=>r.name))].sort().join(' '));}
