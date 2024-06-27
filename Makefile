all:

	mkdir binaries

	wget -P binaries/ http://dist.gem5.org/dist/v22-0/arm/aarch-system-20220707.tar.bz2
	tar -xf binaries/aarch-system-20220707.tar.bz2 -C binaries

	wget -P binaries/ http://dist.gem5.org/dist/v22-0/arm/disks/ubuntu-18.04-arm64-docker.img.bz2
	bzip2 -d binaries/ubuntu-18.04-arm64-docker.img.bz2 

	mkdir binaries/mnt

	sudo mount -o loop,offset=65536 binaries/ubuntu-18.04-arm64-docker.img binaries/mnt
	sudo cp -r apps/ binaries/mnt/root/
	sudo umount binaries/mnt 
