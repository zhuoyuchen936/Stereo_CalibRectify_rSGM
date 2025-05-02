# Stereo_Calib-Rectify_rSGM


# Env配置
windows11安装WSL2，linux子系统
​WSL2中配置环境变量：

```bash
echo "export DISPLAY=$(awk '/nameserver / {print $2}' /etc/resolv.conf):0" >> ~/.bashrc
source ~/.bashrc
sudo apt install qttools5-dev-tools #处理资源文件（.qrc）

sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential cmake git #安装编译工具

sudo apt install -y qtbase5-dev qttools5-dev-tools libqt5gui5 libqt5core5a libqt5widgets5 #安装qt5开发环境
qmake --version 
# 检查qmake版本，我的是QMake version 3.1
#Using Qt version 5.15.2 in /home/czy/anaconda3/envs/py310/lib

sudo apt install -y libopencv-dev #快速安装opencv
sudo apt-get update
sudo apt-get install libpcl-dev #PCL 核心库和开发文件
#安装pcl依赖
sudo apt-get install git build-essential linux-libc-dev
sudo apt-get install cmake cmake-gui
sudo apt-get install libusb-1.0-0-dev libusb-dev libudev-dev
sudo apt-get install mpi-default-dev openmpi-bin openmpi-common
sudo apt-get install libflann1.9 libflann-dev  # ubuntu20.4对应1.9
sudo apt-get install libeigen3-dev
sudo apt-get install libboost-all-dev
sudo apt-get install libqhull* libgtest-dev
sudo apt-get install freeglut3-dev pkg-config
sudo apt-get install libxmu-dev libxi-dev
sudo apt-get install mono-complete
sudo apt-get install libopenni-dev
sudo apt-get install libopenni2-dev
# 安装 PCL 核心库及其依赖
sudo apt install libpcl-dev pcl-tools
pcl-config --version # 验证安装（查看版本）
conda create -n stereoRec python=3.10
conda install -c conda-forge libstdcxx-ng=12
 
```
# 测试环境
1. Windows端安装[VcXsrv](https://sourceforge.net/projects/vcxsrv/files/latest/download)，实现WSL2与Windows的图形界面交互。
2. 首次启动配置​
    安装完成后，运行 ​XLaunch​：
    
    - 选择 "Multiple windows" → 点击 Next
    - 选择 "Start no client" → 点击 Next
    - 勾选 **Disable access control**和**Native opengl**（关键步骤！允许 WSL2 连接）→ 点击 Next
    - 点击 "Save configuration" 保存配置（可选，方便下次快速启动）
    - 点击 Finish，任务栏会出现 X 图标（表示服务已启动）
    ```bash
    # 获取 Windows 主机的 IP 地址（通过 resolv.conf）
        export DISPLAY=$(awk '/nameserver/ {print $2}' /etc/resolv.conf):0
        # 永久生效（写入 .bashrc 或 .zshrc）
        echo "export DISPLAY=$(awk '/nameserver/ {print $2}' /etc/resolv.conf):0" >> ~/.bashrc
        source ~/.bashrc
        sudo apt install -y x11-apps
        #在 Windows 终端运行 ipconfig，找到wsl的IPv4地址（我的是172.19.32.1）
        export DISPLAY=172.19.32.1:0
        xeyes #出现眼睛图片就成功
    
    ```
3. 运行环境测试程序
```bash
cd Stereo_CalibRectify_rSGM/env_verify/build
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake #确保CMake找到Qt
make -j$(nproc)
./MyProject #出现"Hello Qt + OpenCV!"界面就ok
```
# Usage
1. Run the code in Visual Studio 2022.
2. Input the path of the left and right images.
