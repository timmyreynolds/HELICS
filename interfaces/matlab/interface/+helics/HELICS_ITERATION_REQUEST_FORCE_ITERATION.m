function v = HELICS_ITERATION_REQUEST_FORCE_ITERATION()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 135);
  end
  v = vInitialized;
end
