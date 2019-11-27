#include "MPxNodeTemplatePlugin.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj, "My plug-in", "1.0", "Any");
	status = plugin.registerNode("MPxNodeTemplatePlugin", MPxNodeTemplatePlugin::id, MPxNodeTemplatePlugin::creator, MPxNodeTemplatePlugin::initialize);
	return status;
}

MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj);
	status = plugin.deregisterNode(MPxNodeTemplatePlugin::id);
	return status;
}