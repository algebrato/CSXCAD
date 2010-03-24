function CSX = AddProbe(CSX, name, type)

if ~ischar(name)
    error('CSXCAD::AddProbe: name must be a string');
end

if isfield(CSX.Properties,'ProbeBox')
    CSX.Properties.ProbeBox{end+1}.ATTRIBUTE.Name=name;    
else
    CSX.Properties.ProbeBox{1}.ATTRIBUTE.Name=name;
end

CSX.Properties.ProbeBox{end}.ATTRIBUTE.Type=type;