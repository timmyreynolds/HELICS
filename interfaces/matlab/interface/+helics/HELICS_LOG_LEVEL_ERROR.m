function v = HELICS_LOG_LEVEL_ERROR()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 59);
  end
  v = vInitialized;
end
