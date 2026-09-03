# HƯỚNG DẪN CROSS_COMPILER CHO FORLINX6ULL

## Bước 1. Tải tools chain tại

<https://drive.google.com/drive/folders/1xw-Jpw6huxsY_Zlq5Bet-IsloFrmhlpt?usp=drive_link>

## Bước 2. sau khi tải song vào thư mục đã tải:

dùng lệnh này để cấp quyền thực thi và chạy file `.sh` để cài toolchain

```bash
sudo chmod +x fsl-imx-fb-glibc-x86_64-core-image-minimal-cortexa7t2hf-neon-okmx6ull-s-emmc-toolchain-5.15-kirkstone.sh

./fsl-imx-fb-glibc-x86_64-core-image-minimal-cortexa7t2hf-neon-okmx6ull-s-emmc-toolchain-5.15-kirkstone.sh
```

## Bước 3 thêm toolchain vào biến môi trường

dùng lệnh này để add vào môi trường

```bash
source /opt/fsl-imx-fb/5.15-kirkstone/environment-setup-cortexa7t2hf-neon-poky-linux-gnueabi
```

## Bước 4 có thể dùng make hoặc biến môi trường để build

vd:

```bash
$CC hello.c -o hello
```

## Bước 5 deploy sang board

board em để nó tự kết nối vào wifi

```text
ssid: Hunonic wifi 32 Ky Tu
```

username của board: `root`

*không có password*

> anh scp sang chạy thử

nếu anh cần dùng mqtt thì em build sẵn cho board dùng lib mqtt mosquito rồi
