Pod::Spec.new do |s|
  s.name = 'DobsonNativePlugin'
  s.version = '1.0.0'
  s.summary = 'Native Wi-Fi, motion sensors and SQLite catalogue for Dobson Push-To.'
  s.license = { :type => 'MIT' }
  s.author = 'Dobson Push-To'
  s.homepage = 'https://github.com/prasfabien-sudo/PushTo'
  s.source = { :git => 'https://github.com/prasfabien-sudo/PushTo.git', :branch => 'main' }
  s.source_files = 'Sources/DobsonNativePlugin/**/*.{swift,h,m,mm}'
  s.ios.deployment_target = '15.0'
  s.swift_version = '5.9'
  s.dependency 'Capacitor'
  s.frameworks = 'CoreMotion', 'NetworkExtension'
  s.libraries = 'sqlite3'
end
