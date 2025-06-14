function sysCall_init()
    local sim = require 'sim'
    local simUI = require 'simUI'
    local simIGL = require 'simIGL'
    local sel = sim.getObjectSel()
    if #sel ~= 1 or sim.getObjectType(sel[1]) ~= sim.sceneobject_shape then
        simUI.msgBox(
            simUI.msgbox_type.critical, simUI.msgbox_buttons.ok, 'Mesh subdivision add-on',
            'This tool requires exactly one shape to be selected.'
        )
    else
        local m = simIGL.upsample(simIGL.getMesh(sel[1]))
        local h = sim.createShape(3, math.pi / 8, m.vertices, m.indices)
        sim.relocateShapeFrame(h, {0, 0, 0, 0, 0, 0, 0})
        sim.setObjectColor(h, 0, sim.colorcomponent_ambient_diffuse, {0.85, 0.96, 1.0})
        sim.setObjectFloatParam(h, sim.shapefloatparam_edge_angle, 0)
        sim.setObjectFloatParam(h, sim.shapefloatparam_shading_angle, 0)
        sim.setObjectInt32Param(h, sim.shapeintparam_edge_visibility, 1)
        sim.setObjectInt32Param(h, sim.shapeintparam_culling, 0)
        sim.announceSceneContentChange()
        sim.setObjectSel({h})
    end
    return {cmd = 'cleanup'}
end

function sysCall_info()
    return {autoStart = false, menu = 'Geometry / Mesh\nMesh subdivide'}
end
