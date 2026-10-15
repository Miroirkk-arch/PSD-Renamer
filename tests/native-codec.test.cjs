const test=require('node:test'),assert=require('node:assert/strict'),fs=require('fs'),path=require('path'),crypto=require('crypto');const {readPsd,initializeCanvas}=require('ag-psd');
initializeCanvas(()=>{throw Error('canvas');},(width,height)=>({width,height,data:new Uint8ClampedArray(width*height*4)}));
test('native PSD save retains original layer pixels, masks, opacity and clipping after moves',()=>{
 const source=path.join(__dirname,'.native','样例.psd'),staged=path.join(__dirname,'..','release','PSD-Renamer','test-results','native-output.psd');
 const output=process.env.PSD_NATIVE_OUTPUT||(fs.existsSync(staged)?staged:path.join(__dirname,'.native','results','native-output.psd'));
 const read=file=>readPsd(fs.readFileSync(file),{useImageData:true,useRawThumbnail:true});const before=read(source),after=read(output);
 const index=doc=>{const map=new Map();const walk=nodes=>nodes.forEach(n=>{if(n.children)walk(n.children);else map.set(n.name,n);});walk(doc.children);return map;};const a=index(before),b=index(after);
 assert.equal(after.width,before.width);assert.equal(after.height,before.height);
 for(const [name,n] of a){const saved=b.get(name);assert.ok(saved,name);assert.equal(saved.clipping,n.clipping);assert.equal(saved.blendMode,n.blendMode);assert.equal(saved.opacity,n.opacity);if(n.imageData)assert.deepEqual(saved.imageData.data,n.imageData.data,name);if(n.mask)assert.deepEqual(saved.mask.imageData.data,n.mask.imageData.data,name+' mask');}
 assert.ok(after.imageData?.data.length===100*100*4);assert.equal(after.children.some(n=>n.name==='眼睛'),true,'selected layer moved to root');
 const created=after.children.find(n=>n.name==='新建图层');assert.ok(created,'new layer saved');assert.equal(created.children,undefined);assert.equal(created.right-created.left,0);assert.equal(created.bottom-created.top,0);assert.ok(!created.imageData||created.imageData.data.length===0,'new layer remains genuinely empty');
 const group=after.children.find(n=>n.name==='五官');assert.ok(group.hidden);for(const n of group.children)assert.ok(n.hidden);
 console.log('source SHA256',crypto.createHash('sha256').update(fs.readFileSync(source)).digest('hex'));
});
