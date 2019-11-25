#include "TemplatePlugin.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj, "My plug-in", "1.0", "Any");
	status = plugin.registerNode("TemplatePlugin", TemplatePlugin::id, TemplatePlugin::creator, TemplatePlugin::initialize);
	return status;
}

MStatus uninitializePlugin(MObject obj) {
	MStatus status;
	MFnPlugin plugin(obj);
	status = plugin.deregisterNode(TemplatePlugin::id);
	return status;
}