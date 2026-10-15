const fs=require('fs'),path=require('path');
const directory=path.join(__dirname,'icons');fs.mkdirSync(directory,{recursive:true});
const icons={eye:'EyeOutlined',hidden:'EyeInvisibleOutlined',expand:'PlusSquareOutlined',collapse:'MinusSquareOutlined',folder:'FolderOutlined',folderOpen:'FolderOpenOutlined',new:'FolderAddOutlined',layer:'FileAddOutlined',moon:'MoonOutlined',sun:'SunOutlined',check:'BorderOutlined',checked:'CheckSquareOutlined',save:'SaveOutlined',book:'BookOutlined',trash:'DeleteOutlined',minus:'MinusOutlined',plus:'PlusOutlined',maximize:'BorderOutlined',close:'CloseOutlined',settings:'SettingOutlined'};
const escape=value=>String(value).replaceAll('&','&amp;').replaceAll('"','&quot;').replaceAll('<','&lt;');
function render(node){const attrs=Object.entries(node.attrs||{}).filter(([key])=>!['focusable'].includes(key)).map(([key,value])=>` ${key}="${escape(value)}"`).join('');return `<${node.tag}${attrs}>${(node.children||[]).map(render).join('')}</${node.tag}>`;}
for(const [key,name] of Object.entries(icons)){const definition=require('@ant-design/icons-svg/lib/asn/'+name).default.icon;fs.writeFileSync(path.join(directory,key+'.svg'),render(definition));}
fs.writeFileSync(path.join(__dirname,'icons.qrc'),'<RCC><qresource prefix="/icons">'+Object.keys(icons).map(key=>`<file alias="${key}.svg">icons/${key}.svg</file>`).join('')+'</qresource></RCC>');
