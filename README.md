## ☁️ fcloud-command
2023-1 system programming team project
---
* server 오픈 시 mysql 해당 로그인 계정 권한을 열어 두어야 함
  '''mysql
  grant all privileges on *.* to 'id'@'localhost/@' identified by 'password'
  '''
* client가 서버 접속 위해서 server는 port와 ip 주소 제공
