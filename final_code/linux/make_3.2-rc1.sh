#!/bin/bash
cd ./linux-3.2-rc1
make -j4 1>/dev/null
make modules modules_install install 1>/dev/null
update-grub
