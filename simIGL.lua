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

return simIGL
