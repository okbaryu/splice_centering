# fi-splice-centering-con
repository of fi-splice-centering-con

1. the way of getting source

git clone git@github.com:hssysco/fi-splice-centering-con.git folder_name
또는
git clone https://github.com/hssysco/fi-splice-centering-con folder_name

ex) git clone git@github.com:hssysco/fi-splice-centering-con.git ipu

ex) git clone git@github.com:hssysco/fi-splice-centering-con.git splice

2. the way of compiling bootloader

cd ipu/lichee/brandy

./build.sh -p sun8iw8p1

./build.sh -p sun8iw8p1_nor

3. the way of compiling lichee

cd ipu/lichee

./build.sh 

4. the way of making image

cd ipu/lichee

./build.sh pack

5. the way of clean lichee

cd ipu/lichee

./build.sh clean

6. the way of distclean lichee

cd ipu/lichee

./build.sh distclean

7. the way of distclean kernel

cd ipu/lichee/linux-3.4

make distclean


