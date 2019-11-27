#include "MPxDeformerNodeTemplatePlugin.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj, "My plug-in", "1.0", "Any");
	status = plugin.registerNode("MPxDeformerNodeTemplatePlugin", MPxDeformerNodeTemplatePlugin::id, MPxDeformerNodeTemplatePlugin::creator, MPxDeformerNodeTemplatePlugin::initialize);
	return status;
}

MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj);
	status = plugin.deregisterNode(MPxDeformerNodeTemplatePlugin::id);
	return status;
}