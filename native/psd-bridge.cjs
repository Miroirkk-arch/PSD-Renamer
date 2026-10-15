// Short-lived PSD codec. No Electron or browser is loaded by the native application.
const fs = require('fs');
const path = require('path');
const { readPsd, writePsdBuffer, initializeCanvas } = require('ag-psd');
initializeCanvas(() => { throw Error('Canvas decoding disabled'); }, (width, height) => ({ width, height, data: new Uint8ClampedArray(width * height * 4) }));
const [operation, input, workspace, output] = process.argv.slice(2);
function read(file) { return readPsd(fs.readFileSync(file), { useImageData: true, useRawThumbnail: true, skipCompositeImageData: true }); }
function pixels(image, name) {
  if (!image?.data || !image.width || !image.height) return null;
  let left = image.width, top = image.height, right = 0, bottom = 0;
  for (let y = 0; y < image.height; y++) for (let x = 0; x < image.width; x++) if (image.data[(y * image.width + x) * 4 + 3]) {
    left = Math.min(left, x); top = Math.min(top, y); right = Math.max(right, x + 1); bottom = Math.max(bottom, y + 1);
  }
  if (right) fs.writeFileSync(path.join(workspace, name), Buffer.from(image.data.buffer, image.data.byteOffset, image.data.byteLength));
  return { file: name, width: image.width, height: image.height, crop: right ? [left, top, right - left, bottom - top] : [0, 0, 0, 0] };
}
try {
  if (operation === 'open') {
    fs.mkdirSync(workspace, { recursive: true });
    const document = read(input);
    const walk = (children, parent = '') => children.map((layer, index) => {
      const id = parent + '/' + index, key = id.replaceAll('/', '-');
      const image = pixels(layer.imageData, key + '.rgba');
      const mask = layer.mask ? { ...layer.mask, imageData: undefined, image: pixels(layer.mask.imageData, key + '-mask.rgba') } : null;
      return { id, name: layer.name || '未命名图层', group: Array.isArray(layer.children), hidden: !!layer.hidden,
        opened: layer.opened !== false, left: layer.left || 0, top: layer.top || 0, right: layer.right || 0, bottom: layer.bottom || 0,
        opacity: layer.opacity ?? 1, fillOpacity: layer.fillOpacity ?? 1, blend: layer.blendMode || 'normal', clipping: !!layer.clipping,
        image, mask, children: layer.children ? walk(layer.children, id) : [] };
    }).reverse();
    const stat = fs.statSync(input);
    fs.writeFileSync(path.join(workspace, 'document.json'), JSON.stringify({ width: document.width, height: document.height, source: { size: stat.size, modified: stat.mtimeMs }, children: walk(document.children || []) }));
    process.stdout.write(JSON.stringify({ ok: true }));
  } else if (operation === 'save') {
    const opened = JSON.parse(fs.readFileSync(path.join(workspace, 'document.json'), 'utf8')), stat = fs.statSync(input);
    if (opened.source && (opened.source.size !== stat.size || opened.source.modified !== stat.mtimeMs)) throw Error('源文件已改变，请重新打开后保存。');
    const document = read(input), manifest = JSON.parse(fs.readFileSync(path.join(workspace, 'save.json'), 'utf8'));
    const sources = new Map();
    const index = (children, parent = '') => children.forEach((layer, i) => { const id = parent + '/' + i; sources.set(id, layer); if (layer.children) index(layer.children, id); });
    index(document.children || []);
    const walk = nodes => nodes.slice().reverse().map(node => {
      const original = sources.get(node.id);
      if (!original && !node.group && !node.id.startsWith('layer-')) throw Error('Missing original layer: ' + node.id);
      const created = node.group ? { blendMode: 'pass through', opacity: 1 } : { blendMode: 'normal', opacity: 1, left: 0, top: 0, right: 0, bottom: 0 };
      const layer = { ...(original || created), name: node.name, hidden: node.hidden };
      if (node.group) { layer.opened = node.opened; layer.children = walk(node.children); }
      return layer;
    });
    document.children = walk(manifest.children);
    const merged = fs.readFileSync(path.join(workspace, 'merged.rgba'));
    if (merged.length !== document.width * document.height * 4) throw Error('Invalid merged pixels');
    document.imageData = { width: document.width, height: document.height, data: new Uint8ClampedArray(merged.buffer, merged.byteOffset, merged.byteLength) };
    delete document.canvas;
    if (document.imageResources) { delete document.imageResources.thumbnail; delete document.imageResources.thumbnailRaw; }
    document.imageResources = { ...document.imageResources, versionInfo: { hasRealMergedData: false, writerName: 'PSD-Renamer', readerName: 'PSD-Renamer', fileVersion: 1 } };
    const bytes = writePsdBuffer(document, { generateThumbnail: false });
    const fd = fs.openSync(output, 'wx');
    try { fs.writeFileSync(fd, bytes); } catch (error) { fs.closeSync(fd); fs.unlinkSync(output); throw error; }
    fs.closeSync(fd);
    process.stdout.write(JSON.stringify({ ok: true, output }));
  } else throw Error('Unknown operation');
} catch (error) { process.stderr.write(error.stack || error.message); process.exitCode = 1; }
