#!/bin/bash
port=$1
if [ -z "$port" ]; then
  echo "Usage: $0 <port>"
  exit 1
fi
if curl -s -o /dev/null -w "%{http_code}" --connect-timeout 3 http://localhost:$port 2>/dev/null | grep -q "^[23]"; then
  echo "UP"
else
  echo "DOWN"
fi
