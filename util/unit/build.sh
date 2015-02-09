
mkdir -p build/
if [ $? -ne 0 ]
then
	echo Unable to create the build directory
	exit -1
fi

cp ConfigUnitConvertor.java build/
cd build

javac ConfigUnitConvertor.java

echo Manifest-Version: 1.0			 > MANIFEST.MF
echo Class-Path: .					 > MANIFEST.MF
echo Main-Class: ConfigUnitConvertor > MANIFEST.MF

jar -cvmf MANIFEST.MF ConfigUnitConvertor.jar *.class 
mv ConfigUnitConvertor.jar ..
cd ..
rm -r build/
