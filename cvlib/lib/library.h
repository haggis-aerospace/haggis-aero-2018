#ifndef CVLIB_LIBRARY_H
#define CVLIB_LIBRARY_H

void imgRecTest();

extern "C"
{
    void hello();
    void libtest();
	void camTest();
	void runCalibration(bool showResult);
}


#endif
