#! /usr/bin/sh
start_nginx(){
  nginx_flag=$(ps | grep nginx | grep -v "grep")
  if [ -n "$nginx_flag" ]; then
    killall -9 nginx
  fi
  cd webservice
  chmod +x ./sbin/nginx
  ./sbin/nginx -p .
  cd -
}
start_nginx