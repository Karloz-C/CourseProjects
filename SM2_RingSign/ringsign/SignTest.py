from subprocess import *
import time

size = '10'
key_id = '0'
msg = 'hello'

start=time.time()
p1 = run(['./ringsign.exe',size,key_id,'0',msg],capture_output=True)
end=time.time()
t1=end-start



print('message:',msg)
sign = str(p1.stdout)[7:-2].split(' ')
print('sign:')
for i in sign:
    print(i)
arg = ['./ringsign.exe',size,key_id,'1',msg]
for i in sign:
    arg.append(i)

start=time.time()
p2 = run(arg,capture_output=True,check=True)
end=time.time()
t2=end-start

print(str(p2.stdout)[2:-1])

print('run time of sign:%fs'% t1)
print('run time of verify:%fs'% t2)