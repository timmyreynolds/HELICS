function v = HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 103);
  end
  v = vInitialized;
end