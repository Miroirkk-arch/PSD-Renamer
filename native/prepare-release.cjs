const fs=require('node:fs');const path=require('node:path');const cp=require('node:child_process');const crypto=require('node:crypto');
const {version,compare,jsonFile,inspectZip,notes}=require('./release-utils.cjs');
const root=path.resolve(__dirname,'..'),args=process.argv.slice(2),mode=args[0]==='draft'?'draft':'prepare';
function option(key){const i=args.indexOf(key);return i<0?null:args[i+1];}
function run(exe,argv,opts={}) {const result=cp.spawnSync(exe,argv,{cwd:root,encoding:'utf8',windowsHide:true,maxBuffer:20*1024*1024,...opts});if(result.error||result.status!==0)throw Error(`${path.basename(exe)} failed\n${result.stderr||result.stdout||result.error}`);return result.stdout;}
function powershell(file,argv=[]){return run('powershell.exe',['-NoProfile','-ExecutionPolicy','Bypass','-File',path.join(__dirname,file),...argv]);}
function smoke(exe,output,fixture,env=process.env){run(exe,['--self-test',fixture],{timeout:90000,env});const result=jsonFile(output);if(!result.pass)throw Error('Native verification failed: '+result.error);return result;}
async function draft(ver,repo,directory,report) {
 if(fs.existsSync(path.join(directory,"DRAFT.json")))throw Error("A draft has already been recorded for this version. Review it before any further upload.");
 if(report.version!==ver||report.repository!==repo||report.archive!==`PSDNaming-v${ver}-win-x64.zip`)throw Error('Release report/version/repository mismatch');
 const zip=path.join(root,'release',report.archive),hash=inspectZip(zip,repo).sha256;if(hash!==report.sha256||!report.ready)throw Error('Prepared archive changed or verification incomplete');
 const asset=path.join(directory,'SHA256SUMS.txt'),body=path.join(directory,'RELEASE.md');
 const gh=cp.spawnSync('gh',['--version'],{encoding:'utf8',windowsHide:true});
 if(!gh.error&&gh.status===0){run('gh',['release','create','v'+ver,zip,asset,'--repo',repo,'--draft','--target','main','--title',`PSD-Renamer v${ver} · PSD命名修改`,'--notes-file',body]);return;}
 const token=process.env.GH_TOKEN||process.env.GITHUB_TOKEN;if(!token)throw Error('Draft not uploaded: GitHub CLI login or GH_TOKEN is required. Prepared files are ready; use the signed-in GitHub release page. Do not paste credentials into chat.');
 async function request(url,method,data,type='application/json'){const response=await fetch(url,{method,headers:{Authorization:'Bearer '+token,Accept:'application/vnd.github+json','User-Agent':'PSDNaming-release','Content-Type':type},body:Buffer.isBuffer(data)?data:JSON.stringify(data),signal:AbortSignal.timeout(60000)});if(!response.ok)throw Error('GitHub request failed: '+response.status);return response.json();}
 // Only create drafts. Never publish, replace an existing asset, or delete releases.
 const release=await request(`https://api.github.com/repos/${repo}/releases`,'POST',{tag_name:'v'+ver,target_commitish:'main',name:`PSD-Renamer v${ver} · PSD命名修改`,body:fs.readFileSync(body,'utf8'),draft:true,prerelease:false});
 fs.writeFileSync(path.join(directory,'DRAFT.json'),JSON.stringify({id:release.id,url:release.html_url,complete:false},null,2));
 for(const file of [zip,asset])await request(release.upload_url.split('{')[0]+'?name='+encodeURIComponent(path.basename(file)),'POST',fs.readFileSync(file),file.endsWith('.zip')?'application/zip':'text/plain');
 fs.writeFileSync(path.join(directory,'DRAFT.json'),JSON.stringify({id:release.id,url:release.html_url,complete:true},null,2));
}
(async()=>{
 const pkgPath=path.join(root,'package.json'),lockPath=path.join(root,'package-lock.json'),pkg=jsonFile(pkgPath);const ver=option('--version')||pkg.version;version(ver);
 const repo=jsonFile(path.join(__dirname,'update-source.json')).repository;if(!/^[A-Za-z0-9-]+\/[A-Za-z0-9_.-]+$/.test(repo))throw Error('Configure the update repository first');
 const directory=path.join(root,'release','prepared-v'+ver),archive=`PSDNaming-v${ver}-win-x64.zip`;
 if(mode==='draft'){await draft(ver,repo,directory,jsonFile(path.join(directory,'READY.json')));console.log('Draft uploaded. Publish only after final review.');return;}
 if(compare(ver,pkg.version)<0)throw Error('Version cannot go backwards');
 if((fs.existsSync(path.join(root,'release',archive))||fs.existsSync(directory))&&!(args.includes('--resume')&&!fs.existsSync(path.join(directory,'READY.json'))))throw Error('Version already prepared. Use --resume only for an incomplete preparation; completed releases are never overwritten.');
 const changes=jsonFile(option('--notes')?path.resolve(root,option('--notes')):path.join(__dirname,'release-notes',ver+'.json'));notes(ver,repo,changes,'',0);
 try{const current=await fetch(`https://api.github.com/repos/${repo}/releases/latest`,{headers:{'User-Agent':'PSDNaming-release'},signal:AbortSignal.timeout(15000)});if(current.ok){const published=await current.json();const latest=published.tag_name.replace(/^v/,'');if(compare(ver,latest)<=0)throw Error('Version must be newer than the public latest release');}else if(current.status!==404)throw Error('Release lookup failed: '+current.status);}catch(error){throw Error('Cannot verify published version: '+error.message);}
 fs.mkdirSync(directory,{recursive:true});const lock=jsonFile(lockPath);pkg.version=lock.version=ver;if(lock.packages?.[''])lock.packages[''].version=ver;fs.writeFileSync(pkgPath,JSON.stringify(pkg,null,2)+'\n');fs.writeFileSync(lockPath,JSON.stringify(lock,null,2)+'\n');
 console.log('Building v'+ver);fs.writeFileSync(path.join(directory,'build.log'),powershell('build.ps1'));
 run(process.execPath,['--test','tests/release-utils.test.cjs']);run(process.execPath,['tests/native-fixture.cjs']);
 const fixture=path.join(root,'tests','.native','样例.psd'),stage=path.join(root,'release-native');
 console.log('Verifying native application');const native=smoke(path.join(stage,'PSD命名修改.exe'),path.join(stage,'test-results','smoke.json'),fixture);
 fs.writeFileSync(path.join(directory,'codec-test.log'),run(process.execPath,['--test','tests/native-codec.test.cjs']));
 powershell('package-release.ps1');const zip=path.join(root,'release',archive),checked=inspectZip(zip,repo);
 console.log('Verifying extracted runtime in an isolated folder');const verification=path.join(root,'native-build','verification-'+ver+'-'+crypto.randomUUID());
 powershell('verify-release.ps1',['-Archive',zip,'-Destination',verification]);const cleanEnv={...process.env,PATH:(process.env.SystemRoot||'C:\\Windows')+'\\System32'};
 const isolated=smoke(path.join(verification,'PSD命名修改','PSD命名修改.exe'),path.join(verification,'PSD命名修改','test-results','smoke.json'),fixture,cleanEnv);
 fs.writeFileSync(path.join(directory,'RELEASE.md'),notes(ver,repo,changes,checked.sha256,checked.size));fs.writeFileSync(path.join(directory,'SHA256SUMS.txt'),checked.sha256+'  '+archive+'\n');
 const report={ready:true,version:ver,repository:repo,archive,sha256:checked.sha256,size:checked.size,entries:checked.entries,nativePassed:native.pass,codecPassed:true,isolatedPassed:isolated.pass,cleanComputerTested:false,verificationDirectory:verification,created:new Date().toISOString()};fs.writeFileSync(path.join(directory,'READY.json'),JSON.stringify(report,null,2));
 console.log('Ready: '+directory);if(args.includes('--draft'))await draft(ver,repo,directory,report);
})().catch(error=>{console.error(error.message);process.exitCode=1;});
