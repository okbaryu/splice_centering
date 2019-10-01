/usr/sbin/wpa_supplicant -iwlan0 -Dnl80211 -c/data/tmp/ipu_wpa.conf &
sleep 3
/sbin/udhcpc -i wlan0
