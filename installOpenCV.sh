#install dependencies

#tesseract dependencies
sudo apt-get install g++
sudo apt-get install autoconf automake libtool
sudo apt-get install autoconf-archive
sudo apt-get install pkg-config
sudo apt-get install libpng12-dev
sudo apt-get install libjpeg8-dev
sudo apt-get install libtiff5-dev
sudo apt-get install zlib1g-dev
sudo apt-get install libicu-dev
sudo apt-get install libpango1.0-dev
sudo apt-get install libcairo2-dev

#opencv dependencies
sudo apt-get install -y build-essential cmake
sudo apt-get install -y qt5-default libvtk6-dev
sudo apt-get install -y zlib1g-dev libjpeg-dev libwebp-dev libpng-dev libtiff5-dev libjasper-dev libopenexr-dev libgdal-dev
sudo apt-get install -y libdc1394-22-dev libavcodec-dev libavformat-dev libswscale-dev libtheora-dev libvorbis-dev libxvidcore-dev libx264-dev yasm libopencore-amrnb-dev libopencore-amrwb-dev libv4l-dev libxine2-dev
sudo apt-get install -y libtbb-dev libeigen3-dev
sudo apt-get install -y python-dev python-tk python-numpy python3-dev python3-tk python3-numpy
sudo apt-get install -y ant default-jdk
sudo apt-get install -y doxygen
sudo apt-get install -y unzip wget

#install leptonica (tesseract dependency)
git clone https://github.com/danbloomberg/leptonica.git leptonica
cd leptonica
mkdir build
cd build 
cmake ..
make
sudo make install
cd ../..

#install tesseract
git clone https://github.com/tesseract-ocr/tesseract.git tesseract-ocr
cd tesseract-ocr
mkdir build
cd build 
cmake ..
make
sudo make install
sudo ldconfig
cd ../..

#install opencv
wget https://github.com/opencv/opencv/archive/3.2.0.zip
unzip 3.2.0.zip
rm 3.2.0.zip
mv opencv-3.2.0 OpenCV
cd OpenCV
wget https://github.com/opencv/opencv_contrib/archive/3.2.0.zip
unzip 3.2.0.zip
rm 3.2.0.zip
mv opencv_contrib-3.2.0 contrib
mkdir build
cd build
cmake -DWITH_OPENGL=ON -DWITH_GDAL=ON -DWITH_XINE=ON -DBUILD_EXAMPLES=ON -DENABLE_PRECOMPILED_HEADERS=OFF -DOPENCV_EXTRA_MODULES_PATH=../contrib/modules ..
make -j4
sudo make install
sudo ldconfig
