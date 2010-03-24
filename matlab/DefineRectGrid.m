function CSX = DefineRectGrid(CSX, deltaUnit, mesh);

CSX.RectilinearGrid.ATTRIBUTE.DeltaUnit = deltaUnit;
CSX.RectilinearGrid.XLines = mesh.x;
CSX.RectilinearGrid.YLines = mesh.y;
CSX.RectilinearGrid.ZLines = mesh.z;