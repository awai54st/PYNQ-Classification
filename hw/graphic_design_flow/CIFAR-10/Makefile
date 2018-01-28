compile_graphical:
	mkdir build;cd build;\
	cp -r ../fc_CIF_0_1 .;cd fc_CIF_0_1;vivado_hls -f script.tcl;cd ..;\
	cp -r ../fc_CIF_0_2 .;cd fc_CIF_0_2;vivado_hls -f script.tcl;cd ..;\
	cp -r ../maxpool_CIF_0_1 .;cd maxpool_CIF_0_1;vivado_hls -f script.tcl;cd ..;\
	cp -r ../maxpool_CIF_0_2 .;cd maxpool_CIF_0_2;vivado_hls -f script.tcl;cd ..;\
	cp -r ../maxpool_CIF_0_3 .;cd maxpool_CIF_0_3;vivado_hls -f script.tcl;cd ..;\
	cp -r ../SCIG_CIF_0_1 .;cd SCIG_CIF_0_1;vivado_hls -f script.tcl;cd ..;\
	cp -r ../SCIG_CIF_0_2 .;cd SCIG_CIF_0_2;vivado_hls -f script.tcl;cd ..;\
	cp -r ../SCIG_CIF_0_3 .;cd SCIG_CIF_0_3;vivado_hls -f script.tcl;cd ..;\
	cp -r ../SMM_CIF_0_1 .;cd SMM_CIF_0_1;vivado_hls -f script.tcl;cd ..;\
	cp -r ../SMM_CIF_0_2 .;cd SMM_CIF_0_2;vivado_hls -f script.tcl;cd ..;\
	cp -r ../SMM_CIF_0_3 .;cd SMM_CIF_0_3;vivado_hls -f script.tcl;cd ..;\
	cp -r ../SKETCHPAD .;cd SKETCHPAD;vivado -source sketchpad.tcl
clean:
	rm -r build
