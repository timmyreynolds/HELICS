function v = HELICS_ITERATION_REQUEST_NO_ITERATION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 127);
  end
  v = vInitialized;
end
