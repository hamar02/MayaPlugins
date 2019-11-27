#include "MPxGPUDeformerNodeTemplatePlugin.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj, "My plug-in", "1.0", "Any");
	status = plugin.registerNode("MPxGPUDeformerNodeTemplatePlugin", MPxGPUDeformerNodeTemplatePlugin::id, MPxGPUDeformerNodeTemplatePlugin::creator, MPxGPUDeformerNodeTemplatePlugin::initialize);
	return status;
}

MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj);
	status = plugin.deregisterNode(MPxGPUDeformerNodeTemplatePlugin::id);
	return status;
}