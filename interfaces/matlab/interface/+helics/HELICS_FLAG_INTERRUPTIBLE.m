function v = HELICS_FLAG_INTERRUPTIBLE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 32);
  end
  v = vInitialized;
end
