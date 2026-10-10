import subprocess,time,json,statistics
res=[]
for mb in [1,8,32,64,128,256]:
    c1=f'head -c {mb*1048576} /dev/zero'
    tt=[];tb=[]
    for _ in range(5):
        t=time.perf_counter(); subprocess.run(['../pipetool'],input=f'{c1}\nwc -c\n',capture_output=True,text=True); tt.append(time.perf_counter()-t)
        t=time.perf_counter(); subprocess.run(['bash','-c',f'{c1} | wc -c'],capture_output=True); tb.append(time.perf_counter()-t)
    a=statistics.median(tt); b=statistics.median(tb)
    res.append(dict(mb=mb,tool=a,bash=b,tool_mbps=mb/a,bash_mbps=mb/b))
    print(mb,round(a*1000,1),round(b*1000,1),round(mb/a),round(mb/b))
json.dump(res,open('results.json','w'),indent=2)
