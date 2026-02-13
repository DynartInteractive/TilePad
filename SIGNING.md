# Code Signing Guide for TilePad

## Azure Trusted Signing Setup

### Account Details

- **Subscription**: Azure subscription 1 (`5aa3cfb0-a93d-475d-90ee-d0362675f6c0`)
- **Resource Group**: `TilePad-Signing`
- **Signing Account**: `dynart-signing`
- **Account URI**: `https://weu.codesigning.azure.net/`
- **Region**: West Europe
- **User**: `z3r0c00l@dynart.net`

### Roles Assigned

- Artifact Signing Identity Verifier
- Artifact Signing Certificate Profile Signer

### Prerequisites

- Azure CLI: `C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin\az.cmd`
- AzureSignTool: installed via `dotnet tool install --global AzureSignTool`
- .NET SDK 9.0

---

## Once Identity Validation is Completed

### Step 1: Get the Identity Validation ID

In the Azure Portal:
1. Go to **dynart-signing** > **Identity validations**
2. Click on the completed validation
3. Copy the **Identity Validation ID** from the details

### Step 2: Create the Certificate Profile

For production (Public Trust):

```powershell
& "C:\Program Files\Microsoft SDKs\Azure\CLI2\wbin\az.cmd" trustedsigning certificate-profile create `
  --resource-group TilePad-Signing `
  --account-name dynart-signing `
  --profile-name dynart-public `
  --profile-type PublicTrust `
  --identity-validation-id "<YOUR_IDENTITY_VALIDATION_ID>" `
  --include-street-address false `
  --include-postal-code false `
  -o table
```

### Step 3: Log in to Azure

```
az login
```

### Step 4: Sign TilePad.exe

```
AzureSignTool sign -kvu https://weu.codesigning.azure.net/ -kva dynart-signing -kvt dynart-public -tr http://timestamp.digicert.com -td sha256 "C:\Projects\TilePad\_package\TilePad.exe"
```

### Step 5: Rebuild the Installer

```
"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" C:\Projects\TilePad\setup.iss
```

### Step 6: Sign the Installer

```
AzureSignTool sign -kvu https://weu.codesigning.azure.net/ -kva dynart-signing -kvt dynart-public -tr http://timestamp.digicert.com -td sha256 "C:\Projects\TilePad\Output\tilepad-setup-0.5.0.exe"
```

### Step 7: Update the GitHub Release

```
gh release upload v0.5.0 "Output/tilepad-setup-0.5.0.exe" --clobber
```

---

## Full Build & Sign & Release Script

Once everything is set up, the full process is:

```powershell
# 1. Build
cd C:\Projects\TilePad\build
C:\qt\Qt6\Tools\CMake_64\bin\cmake.exe --build . --config Release

# 2. Deploy Qt DLLs
$env:PATH = "C:\qt\Qt6\6.10.2\mingw_64\bin;C:\qt\Qt6\Tools\mingw1310_64\bin;$env:PATH"
C:\qt\Qt6\6.10.2\mingw_64\bin\windeployqt6.exe TilePad.exe

# 3. Copy to _package
Copy-Item TilePad.exe, *.dll -Destination ..\\_package -Force
Copy-Item platforms\* -Destination ..\\_package\\platforms -Force -Recurse
Copy-Item styles\* -Destination ..\\_package\\styles -Force -Recurse
Copy-Item imageformats\* -Destination ..\\_package\\imageformats -Force -Recurse
Copy-Item iconengines\* -Destination ..\\_package\\iconengines -Force -Recurse
Copy-Item generic\* -Destination ..\\_package\\generic -Force -Recurse
Copy-Item tls\* -Destination ..\\_package\\tls -Force -Recurse
Copy-Item translations\* -Destination ..\\_package\\translations -Force -Recurse

# 4. Sign the exe
az login
AzureSignTool sign -kvu https://weu.codesigning.azure.net/ -kva dynart-signing -kvt dynart-public -tr http://timestamp.digicert.com -td sha256 ..\\_package\\TilePad.exe

# 5. Build installer
cd C:\Projects\TilePad
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" setup.iss

# 6. Sign installer
AzureSignTool sign -kvu https://weu.codesigning.azure.net/ -kva dynart-signing -kvt dynart-public -tr http://timestamp.digicert.com -td sha256 Output\\tilepad-setup-0.5.0.exe

# 7. Upload to GitHub release
gh release upload v0.5.0 Output\\tilepad-setup-0.5.0.exe --clobber
```

---

## Switching from Test to Production

When switching from `PublicTrustTest` to `PublicTrust`:
1. Create a new certificate profile with `--profile-type PublicTrust` and the verified identity validation ID
2. Update the `-kvt` parameter in AzureSignTool from `dynart-test` to `dynart-public`
3. Re-sign and re-release
