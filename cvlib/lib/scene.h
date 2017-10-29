//
// Created by David Helekal on 22/10/2017.
//

/**
 * OK so the thing is you instantiate this thing via a library static method, you then add analyzer objects to it
 * which track / detect features in the scene
 * The actual improc is done via a method accepting a frame or a stream thereof
 */

#ifndef CVLIB_SCENEPROCESSOR_H
#define CVLIB_SCENEPROCESSOR_H

#include <opencv2/core/mat.hpp>

struct rect {
    const int x0;
    const int y0;
    const int w;
    const int h;
};

class scene_processor {
public:

    /**
     * set current frame
     * @return 0 if successful, 1 otherwise
     */
    int set_frame(cv::Mat);
    /**
     * process the current frame
     * @param timeout timeout
     * @return 0 if successful 1 if operation timed out 2 in case of an unspecified error
     */
    int process_frame(size_t timeout);
    /**
     *
     * @return objects of interest located within the current frame by the process_frame method
     */
    std::vector<scene_object> get_scene();

    /**
     * 0 ready 1 initialisation required 2 busy
     * @return
     */
    int get_status();

    scene_processor();
    ~scene_processor();
};
/**
 * Describes an object located in the scene
 */
class scene_object {
    /**
     *
     * @return the ROI containing this object
     */
    rect get_bounding_box();
    /**
     * retrieve a property for the object located within this ROI
     * @param key key
     * @return the value for the given property
     */
    char* read_property(char* key);
};
#endif //CVLIB_SCENEPROCESSOR_H
