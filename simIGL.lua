local simIGL={}

--@fun getMesh get mesh data of a given shape in the format used by simIGL functions
--@arg int h the handle of the shape
--@arg {type=table,default={}} options options
--@ret table mesh mesh object
function simIGL.getMesh(h,options)
    local v,i,n=sim.getShapeMesh(h)
    local m=sim.getObjectMatrix(h,-1)
    v=sim.multiplyVector(m,v)
    return {vertices=v, indices=i}
end

--@fun meshBooleanShape convenience wrapper for simIGL.meshBoolean to operate on shapes directly
--@arg table.int handles the handle of the input shapes
--@arg int op the operation (see simIGL.boolean_op)
--@ret int handleResult the handle of the resulting shape
function simIGL.meshBooleanShape(handles,op)
    if #handles<2 then error('not enough input shapes') end
    local meshes=map(simIGL.getMesh,handles)
    local result=simIGL.meshBoolean(meshes[1],meshes[2],op)
    for i=3,#meshes do result=simIGL.meshBoolean(result,meshes[i],op) end
    local edges=sim.getObjectInt32Param(handles[1],sim.shapeintparam_edge_visibility)
    result=sim.createMeshShape(1+2*edges,math.pi/8,result.vertices,result.indices)
    local _,colad=sim.getShapeColor(handles[1],nil,sim.colorcomponent_ambient_diffuse)
    sim.setShapeColor(result,nil,sim.colorcomponent_ambient_diffuse,colad)
    local _,colsp=sim.getShapeColor(handles[1],nil,sim.colorcomponent_specular)
    sim.setShapeColor(result,nil,sim.colorcomponent_specular,colsp)
    local _,colem=sim.getShapeColor(handles[1],nil,sim.colorcomponent_emission)
    sim.setShapeColor(result,nil,sim.colorcomponent_emission,colem)
    sim.reorientShapeBoundingBox(result,sim.handle_world)
    return result
end

return simIGL
