########################################
## generate libqc.so　　in ./ Fortran 
##			nl 2015.01.06
########################################
echo "------------Compiling Fortran code------------"
#source /opt/intel/Compiler/11.0/081/bin/ifortvars.sh intel64
cd ./Fortran 
gfortran  -w -shared -fPIC P108_RADAR_LIB_3D.f P106_MEDIAN.f MSADataFormat.f90 MComDataModule.f90 P107_DIS_STEP.f90 P105_CRESS_MAN.f90 P104_NOISE_FILTER.f90 P103_AP_DETS.f90 P102_READSA88D.f90 P102_READSA.f90 P101_QC.f90 MCDDataFormat.f90 P102_READCD.f90 MCD_SADataFormat.f90 P102_READSCSA.f90 MCCDataFormat.f90 P102_READCC.f90 MCBDataFormat.f90 P10B_REWRITE_BASE_DATA.f90 P10A_REWRITE_BASE_DATA.f90 P102_READCA.f90 ganshe.f90 C101_COMMSUBROUTINE.F90 MCC12DataFormat.f90 READCC12.f90  -o ../libqc.so
echo "fortran generate \"../libQC.so\" done"
#cp libqc.so ../bin
#su root
#cp ../bin/libqc.so /usr/lib/libqc.so
#sudo #cp ../bin/qc_f.so /usr/lib
#cd ..
##Compiling CPP code
#xlC_r -O3 -w `wx-config --version=2.8 --cxxflags --static=no --unicode=yes`  -D__WCHAR_TYPE__=wchar_t -L/usr/local/wxWidgets-2.8.7/lib/ -#D_SHOW_LOG_DLG -o ./bin/RadarMosaic -L/usr/local/lib -L/usr/lib -lwx_gtk2u-2.8 -lqc ./*.o
#echo "link .o done"
#make
#sudo make install
