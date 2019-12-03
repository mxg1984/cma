########################################
## RadarMosaic install.sh 	     ##
## Version1.0 chenxiang 20090617 ##
########################################

##Compiling Fortran code
#source /opt/intel/Compiler/11.0/081/bin/ifortvars.sh intel64
#cd ./Fortran
#g95  -w -shared -fPIC P108_RADAR_LIB_3D.f P106_MEDIAN.f MSADataFormat.f90 MComDataModule.f90 P107_DIS_STEP.f90 P105_CRESS_MAN.f90 P104_NOISE_FILTER.f90 P103_AP_DETS.f90 P102_READSA88D.f90 P102_READSA.f90 P101_QC.f90 MCDDataFormat.f90 P102_READCD.f90 MCD_SADataFormat.f90 P102_READSCSA.f90 MCCDataFormat.f90 P102_READCC.f90 MCBDataFormat.f90 P10B_REWRITE_BASE_DATA.f90 P10A_REWRITE_BASE_DATA.f90 P102_READCA.f90 ganshe.f90 C101_COMMSUBROUTINE.F90 MCC12DataFormat.f90 READCC12.f90 -o libqc.so
#echo "fortran so done"
#cp libqc.so ../bin
#su root
#cp ../bin/libqc.so /usr/lib/libqc.so
#sudo #cp ../bin/qc_f.so /usr/lib
#cd ..
##Compiling CPP code
#xlC_r -O3 -w `wx-config --version=2.8 --cxxflags --static=no --unicode=yes`  -D__WCHAR_TYPE__=wchar_t -L/usr/local/wxWidgets-2.8.7/lib/ -D_SHOW_LOG_DLG -o ./bin/RadarMosaic -L/usr/local/lib -L/usr/lib -lwx_gtk2u-2.8 -lqc ./*.o
#echo "link .o done"
#make
#sudo make install


#xlC_r -c -w `wx-config --version=2.8 --cxxflags --static=no --unicode=yes`  -D__WCHAR_TYPE__=wchar_t -L/usr/local/wxWidgets-2.8.7/lib/ -D_SHOW_LOG_DLG -DUNIX ./RadarMosaic/MsgMemManager.cpp -I/usr/local/include/wx-2.8
echo "------------RadarMosaic begin------------"
echo "please wait..."
cd ./RadarMosaic
#xlC_r  -O3 -w -c `wx-config --version=2.8 --cxxflags --static=no --unicode=yes`  -D__WCHAR_TYPE__=wchar_t -L/usr/local/wxWidgets-2.8.7/lib/ -D_SHOW_LOG_DLG  BaseDataQCPro.cpp DebugLogDlg.h FormRad3DGrudData.cpp MsgMemManager.cpp RadarMosaicApp.cpp DebugLogDlg.cpp DetectBaseDataOldTime.cpp MainFile.cpp MultiMosaic.cpp RefDerivedProduct.cpp -I/usr/local/include/wx-2.8
#xlc++_r  -O3 -w -c `wx-config --version=2.8 --cxxflags --static=no --unicode=yes`  -D__WCHAR_TYPE__=wchar_t -L/usr/local/wxWidgets-2.8.7/lib/ -D_SHOW_LOG_DLG ./*.cpp -I/usr/local/include/wx-2.8
xlc++_r -O3 -w -c -I/usr/local/wxWidgets-2.8.7/lib/wx/include/gtk2-unicode-release-2.8 -I/usr/local/wxWidgets-2.8.7/include/wx-2.8 -D__WXGTK__ -D_THREAD_SAFE  -D__WCHAR_TYPE__=wchar_t -L/usr/local/wxWidgets-2.8.7/lib/ -D_SHOW_LOG_DLG ./*.cpp 
#xlC_r  -O3 -c -w `wx-config --version=2.8 --cxxflags --static=no --unicode=yes`  -D__WCHAR_TYPE__=wchar_t -L/usr/local/wxWidgets-2.8.7/lib/ -D_SHOW_LOG_DLG ./BaseDataQCPro.cpp -I/usr/local/include/wx-2.8
echo "cpp's *.o done"
cd ..
cd ./Fortran/code_for_gfortran
xlf_r  -O3 -w -c -qextname=p01_main_process_qc:p01_main_process_no_qc P108_RADAR_LIB_3D.f P106_MEDIAN.f MSADataFormat.f90 MComDataModule.f90 P107_DIS_STEP.f90 P105_CRESS_MAN.f90 P104_NOISE_FILTER.f90 P103_AP_DETS.f90 P102_READSA88D.f90 P102_READSA.f90 P101_QC.f90 MCDDataFormat.f90 P102_READCD.f90 MCD_SADataFormat.f90 P102_READSCSA.f90 MCCDataFormat.f90 P102_READCC.f90 MCBDataFormat.f90 P10B_REWRITE_BASE_DATA.f90 P10A_REWRITE_BASE_DATA.f90 P102_READCA.f90 ganshe.f90 C101_COMMSUBROUTINE.F90 MCC12DataFormat.f90 READCC12.f90

#ar -r libqc.a C101_COMMSUBROUTINE.o P102_READSA88D.o MCBDataFormat.o P102_READSCSA.o MCC12DataFormat.o P103_AP_DETS.o MCCDataFormat.o P104_NOISE_FILTER.o MCDDataFormat.o P105_CRESS_MAN.o MCD_SADataFormat.o P106_MEDIAN.o MComDataModule.o P107_DIS_STEP.o MSADataFormat.o P108_RADAR_LIB_3D.o P101_QC.o P10A_REWRITE_BASE_DATA.o P102_READCA.o P10B_REWRITE_BASE_DATA.o P102_READCC.o READCC12.o P102_READCD.o ganshe.o P102_READSA.o
echo "fortran's *.o done"
cd ../../
xlc++_r  -O3 -w `wx-config --version=2.8 --cxxflags --static=no --unicode=yes`  -D__WCHAR_TYPE__=wchar_t -L/usr/local/wxWidgets-2.8.7/lib/ -D_SHOW_LOG_DLG ./RadarMosaic/*.o ./Fortran/code_for_gfortran/*.o /usr/lib/libpng.a -L/usr/lib -lwx_gtk2u-2.8 -o ./bin/RadarMosaic -lxlf90
echo "link .o done"

#Remove *.o files
echo "Remove *.o files"
cd ./RadarMosaic
rm *.o
cd ..
cd ./Fortran/code_for_gfortran
rm *.o
cd ../../
echo "------------RadarMosaic end------------"

