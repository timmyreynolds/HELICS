function v = helics_iteration_request_no_iteration()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183116);
  end
  v = vInitialized;
end