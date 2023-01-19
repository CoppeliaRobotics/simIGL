local simIGL={}

function simIGL.getMesh(h,options)
    options=options or {}
    local v,i,n=sim.getShapeMesh(h)
    local m=sim.getObjectMatrix(h,-1)
    v=sim.multiplyVector(m,v)
    return {vertices=v, indices=i}
end

return simIGL
