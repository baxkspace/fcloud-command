## ☁️ fcloud-command ##
#### *2023 system programming term project*
---

* 접속/연결 시 주의 사항
  * server 오픈 시 mysql 해당 로그인 계정 권한을 열어 두어야 함

  ``` mysql
  grant all privileges on *.* to 'id'@'localhost/@' identified by 'password';
  ```
  * client가 서버 접속 위해서 server는 port와 ip 주소 제공
  * server에서 mysql 권한 부여 위해 추가적으로 실행해야 하는 것
  
    * /etc/apparmor.d/local/usr.sbin.mysqld 파일 내로 들어가 */home/개인 우분투 아이디/*** r, 입력
    * /etc/mysql/mysql.conf.d/mysqld.cnf 파일 내로 들어가 [mysql] * Basic Settings 윗줄에 *secure_file_priv="/home/우분투 아이디"* 입력
  * sudo utf allow <포트번호> 로 방화벽 열기
  * 리눅스를 가상머신에서 bridge 모드로 들어가야 함
  * gcc -o 실행파일 코드파일 -I/usr/include/mysql -lmysqlclient -L/usr/lib -lcurses -lpthread
