const fs = require('node:fs');
const zlib = require('node:zlib');
const crypto = require('node:crypto');
function version(value) {
  if (!/^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)$/.test(value)) throw Error('Version must be MAJOR.MINOR.PATCH');
  const parts = value.split('.').map(Number);
  if (parts.some(n => !Number.isSafeInteger(n) || n > 1000000)) throw Error('Version is too large');
  return parts;
}
function compare(a,b) { a=version(a); b=version(b); for(let i=0;i<3;i++) if(a[i]!==b[i]) return Math.sign(a[i]-b[i]); return 0; }
function jsonFile(file) {return JSON.parse(fs.readFileSync(file,'utf8').replace(/^\uFEFF/,''));}
function inspectZip(file,repository) {
  const zip=fs.readFileSync(file); let end=-1;
  for(let i=zip.length-22;i>=Math.max(0,zip.length-65557);i--) if(zip.readUInt32LE(i)===0x06054b50){end=i;break;}
  if(end<0||zip.readUInt16LE(end+4)||zip.readUInt16LE(end+6)) throw Error('Invalid ZIP archive');
  const count=zip.readUInt16LE(end+10); let at=zip.readUInt32LE(end+16); const files=new Map();
  for(let i=0;i<count;i++) {
    if(zip.readUInt32LE(at)!==0x02014b50) throw Error('Invalid ZIP directory');
    const nameLength=zip.readUInt16LE(at+28), extra=zip.readUInt16LE(at+30), comment=zip.readUInt16LE(at+32);
    const name=zip.subarray(at+46,at+46+nameLength).toString('utf8').replace(/\\/g,'/');
    if(!name.startsWith('PSD-Renamer/')||name.includes(':')||name.split('/').some(p=>p==='..'||p==='.')||files.has(name)) throw Error('Unsafe or duplicate ZIP entry: '+name);
    if(/test-results|test-settings|\.profiles\.json|\.psd$|\.psb$|\.log$|save\.json|\.git\//i.test(name)) throw Error('Private/test data in ZIP: '+name);
    files.set(name,{method:zip.readUInt16LE(at+10),compressed:zip.readUInt32LE(at+20),size:zip.readUInt32LE(at+24),offset:zip.readUInt32LE(at+42)});at+=46+nameLength+extra+comment;
  }
  const required=['LICENSE','THIRD_PARTY_NOTICES.md','PSD-Renamer.exe','Qt6Core.dll','Qt6Gui.dll','Qt6Widgets.dll','Qt6Network.dll','platforms/qwindows.dll','codec/node.exe','codec/psd-bridge.cjs','codec/node_modules/ag-psd/package.json','dictionary.json','update-source.json','licenses/Qt-LGPL-3.0.txt','licenses/Node-LICENSE.txt'];
  for(const name of required) if(!files.has('PSD-Renamer/'+name)) throw Error('Missing runtime file: '+name);
  const entry=files.get('PSD-Renamer/update-source.json'); if(entry.size>4096) throw Error('Invalid update configuration');
  const offset=entry.offset;if(zip.readUInt32LE(offset)!==0x04034b50)throw Error('Invalid ZIP entry header');
  const start=offset+30+zip.readUInt16LE(offset+26)+zip.readUInt16LE(offset+28),data=zip.subarray(start,start+entry.compressed);
  const text=entry.method===0?data:entry.method===8?zlib.inflateRawSync(data,{maxOutputLength:4096}):null;
  if(!text||JSON.parse(text.toString('utf8').replace(/^\uFEFF/,'')).repository!==repository)throw Error('Wrong update repository in ZIP');
  return {entries:files.size,size:zip.length,sha256:crypto.createHash('sha256').update(zip).digest('hex')};
}
function notes(ver,repository,changes,hash,size) {
  version(ver); if(!changes.zh?.length||!changes.en?.length||changes.zh.some(x=>typeof x!=='string')||changes.en.some(x=>typeof x!=='string'))throw Error('Both Chinese and English release notes are required');
  const file=`PSD-Renamer-v${ver}-win-x64.zip`,base=`https://github.com/${repository}`;
  return `# PSD-Renamer v${ver} \n\n[下载 Windows ZIP / Download Windows ZIP](${base}/releases/download/v${ver}/${file}) · [帮助 / Help](${base}/blob/main/SUPPORT.md)\n\nWindows x64 · ${(size/1048576).toFixed(2)} MB · 中文界面 / Chinese UI\n\n## 中文\n\n${changes.zh.map(x=>'- '+x).join('\n')}\n\n解压完整文件夹后打开 PSD-Renamer.exe；更新前退出旧版。个人字典和设置会保留。\n\n## English\n\n${changes.en.map(x=>'- '+x).join('\n')}\n\nExtract the entire folder and run PSD-Renamer.exe. Close the old app before updating. Personal dictionaries and settings are retained.\n\n## SHA256\n\n\x60\x60\x60text\n${hash}  ${file}\n\x60\x60\x60\n`;
}
module.exports={version,compare,jsonFile,inspectZip,notes};
