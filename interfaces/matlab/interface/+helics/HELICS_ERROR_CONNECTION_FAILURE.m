function v = HELICS_ERROR_CONNECTION_FAILURE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 83);
  end
  v = vInitialized;
end
