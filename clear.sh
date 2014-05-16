#/bin/bash
function pause() {
   read -p "$*"
}
rm DEBUG_SWITCH DEBUG_HOST DEBUG_DNS DEBUG_DNS_TABLE
printf "\n" #new line
make clean
make
printf "\n" #new line
pause 'Press [Enter] to continue..'

clear
./net367
