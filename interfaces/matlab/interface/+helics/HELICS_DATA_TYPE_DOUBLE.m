function v = HELICS_DATA_TYPE_DOUBLE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 17);
  end
  v = vInitialized;
end