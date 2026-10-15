const fs=require('fs'),path=require('path');const {writePsdBuffer,initializeCanvas}=require('ag-psd');
initializeCanvas(()=>{throw Error('canvas');},(width,height)=>({width,height,data:new Uint8ClampedArray(width*height*4)}));
const directory=path.join(__dirname,'.native');fs.mkdirSync(directory,{recursive:true});
const image=(width,height,color)=>({width,height,data:new Uint8ClampedArray(Array.from({length:width*height},()=>color).flat())});
const fixture={width:100,height:100,children:[
 {name:'底层',left:0,top:0,imageData:image(2,2,[30,60,90,128])},
 {name:'五官',opened:false,blendMode:'pass through',children:[{name:'眼睛文件夹',opened:false,blendMode:'pass through',children:[
  {name:'眼睛',left:10,top:20,opacity:.8,imageData:image(3,3,[40,70,100,255]),mask:{left:10,top:20,right:13,bottom:23,defaultColor:255,imageData:image(3,3,[200,200,200,255])}},
  {name:'高光',left:10,top:20,clipping:true,blendMode:'screen',imageData:image(1,1,[120,120,120,255])}
 ]}]},
 {name:'空图层',left:0,top:0},
]};fs.writeFileSync(path.join(directory,'样例.psd'),writePsdBuffer(fixture));console.log(path.join(directory,'样例.psd'));
