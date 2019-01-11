Vagrant.configure("2") do |config| 
    config.vm.box = "ubuntu/trusty64" 
    config.vm.synced_folder ".", "/vagrant" 
    config.vm.provider "virtualbox" do |v|
        v.memory = 4096
        v.cpus = 4
    end

    config.vm.define :dev do |dev|
        dev.vm.network "private_network", ip: "192.168.1.10"
        dev.vm.network "forwarded_port", guest: 8080, host: 8080

        dev.vm.provision :shell, inline: 'sudo apt-get update'
        dev.vm.provision :shell, inline: 'sudo apt-get --assume-yes install g++'
        dev.vm.provision :shell, inline: 'sudo apt-get --assume-yes install build-essential libssl-dev git -y'
        dev.vm.provision :shell, inline: 'git clone https://github.com/wg/wrk.git wrk'
        dev.vm.provision :shell, inline: 'cd wrk && make'
        dev.vm.provision :shell, inline: 'cd wrk && sudo cp wrk /usr/local/bin'
    end
end