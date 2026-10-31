const {fs,path,root,chapters}=require('./api-model.cjs');
const crypto=require('crypto');
const hash=p=>crypto.createHash('sha256').update(fs.readFileSync(p)).digest('hex');
function tokens(s){
 const parts=s.match(/\/\*[\s\S]*?\*\/|\/\/[^\n\r]*|"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|[A-Za-z_]\w*|(?:\d+(?:\.\d*)?|\.\d+)(?:[eEpP][+-]?\d+)?[uUlLfF]*|\S/g)||[];
 return parts.filter(x=>!x.startsWith('//')&&!x.startsWith('/*'));
}
const results=[];
for(const[ch,name]of Object.entries(chapters)){
 const dir=path.join(root,name),src=path.join(root,'references/stm32f446re-learning/Nucleo-F446RE',ch),manifest=JSON.parse(fs.readFileSync(path.join(dir,'UPSTREAM.json'),'utf8'));
 let originalHashes=0,unchanged=0,commentOnly=0,portExceptions=0;
 const currentSourceHashes=[];
 const failures=[];
 for(const f of manifest.Files){const a=path.join(src,f.Path),b=path.join(dir,f.Path);if(/\.[ch]$/.test(f.Path))currentSourceHashes.push({Path:f.Path,SHA256:hash(b)});if(hash(a)!==f.SHA256){failures.push('Reference hash changed '+f.Path);continue;}originalHashes++;
  if(hash(a)===hash(b)){unchanged++;continue;}
  let current=fs.readFileSync(b,'utf8');
  if(ch==='CH11'&&f.Path==='Core/Src/main-ex2.c'){
   // The only deliberate non-comment C adaptation: fill the missing local DMA clock helper.
   const ts=tokens(current),add=tokens('static void MX_DMA_Init(void) { __HAL_RCC_DMA1_CLK_ENABLE(); __HAL_RCC_DMA2_CLK_ENABLE(); }');
   const at=ts.findIndex((_,i)=>ts.slice(i,i+add.length).join('\0')===add.join('\0'));
   if(at<0){failures.push('Expected CH11 EX2 port helper not found');continue;}
   ts.splice(at,add.length);if(ts.join('\0')!==tokens(fs.readFileSync(a,'utf8')).join('\0'))failures.push('Unexpected port tokens '+f.Path);else portExceptions++;
  }else if(/\.[ch]$/.test(f.Path)&&tokens(current).join('\0')===tokens(fs.readFileSync(a,'utf8')).join('\0'))commentOnly++;
  else failures.push('Unexpected content change '+f.Path);
 }
 const inv=JSON.parse(fs.readFileSync(path.join(dir,'artifacts/api-inventory.json'),'utf8')),doc=fs.readFileSync(path.join(dir,'本代码所用API说明.md'),'utf8');
 if((doc.match(/^## API：/gm)||[]).length!==inv.Total)failures.push('API card count mismatch');
 if(inv.APIs.some(a=>a.Name==='HAL_MspInit'||a.Name.endsWith('Callback')))failures.push('Callback definition counted as external API');
 const snippets=/来源：\[([^\]:]+):(\d+)\]\([^\n]+\)[^\n]*\n\n```c\n([\s\S]*?)\n```/g;let hit,exampleChecks=0;
 while((hit=snippets.exec(doc))){const s=fs.readFileSync(path.join(dir,hit[1]),'utf8'),line=Number(hit[2]),start=s.split('\n').slice(0,line-1).join('\n').length+(line>1?1:0);if(!s.slice(start,start+hit[3].length+100).includes(hit[3]))failures.push('Stale example '+hit[1]+':'+line);else exampleChecks++;}
 const row={Project:name,ReferenceFiles:originalHashes,ByteIdentical:unchanged,CommentOnlyFiles:commentOnly,DocumentedPortExceptionFiles:portExceptions,ExternalAPIs:inv.Total,ExampleChecks:exampleChecks,CurrentSourceHashes:currentSourceHashes,Failures:failures};
 fs.writeFileSync(path.join(dir,'artifacts/source-verification.json'),JSON.stringify(row,null,2)+'\n');results.push(row);
 console.log(name,JSON.stringify(row));
}
fs.writeFileSync(path.join(root,'book-timers-CH11/artifacts/chapter-source-results.json'),JSON.stringify(results,null,2)+'\n');
if(results.some(r=>r.Failures.length))process.exit(1);
