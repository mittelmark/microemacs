
class Microemacs09 < Formula
  desc "MicroEmacs 09 Text Editor"
  homepage "https://github.com/mittelmark/microemacs"
  version "24b1"

  if OS.mac?
    url "file:///home/dgroth/workspace/microemacs/brew-macos-091224b2.tar.gz"
    sha256 "macos-binary-sha256"
  elsif OS.linux?
    url "file:///home/dgroth/workspace/microemacs/brew-linux-091224b2.tar.gz"
    sha256 "f3ce85246419cee2afd16ec48586c641f38a5370e439a9445f0b6805a00934e3"
  elsif OS.windows?
    url "file:///home/dgroth/workspace/microemacs/brew-windows-091224b2.tar.gz"
    sha256 "45c6312e149b07f299871fdfdb9df89425e16f01b5067dd40eb624b83ae03619"
  end

  def install
    if OS.mac? || OS.linux?
        bin.install "bin/jmc"
        bin.install "bin/jmw"
    elsif OS.windows?
      system "unzip", "myapp-windows.zip", "-d", bin
    end
  end

  test do
    assert_match "MicroEmacs 09 - Date 2009/12/#{version}", shell_output("#{bin}/mec -V")
  end
end
