function v = helics_iteration_request_iterate_if_needed()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 114);
  end
  v = vInitialized;
end
