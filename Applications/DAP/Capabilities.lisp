(import json)
(import lsp)

(eval-lsp (add-readers *READTABLE*))

(set capabilites 
{
  supportsEvaluateForHovers: true,
  supportsConfigurationDoneRequest: true
}
)

