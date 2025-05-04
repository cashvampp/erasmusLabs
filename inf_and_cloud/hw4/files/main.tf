terraform {
  required_providers {
    openstack = {
      source  = "terraform-provider-openstack/openstack"
      version = "~> 3.0.0"
    }
  }
}

provider "openstack" {}



variable "vm_name" {
  description = "Name of the virtual machine"
  default = "homework4-vm"
}

variable "image_id" {
  description = "Name of the image to use for the VM"
  default     = "033e0a54-423a-4835-afe6-5ae28e82f215" # ubuntu-noble-x86_64
}

variable "flavor_id" {
  description = "Flavor ID for the VM"
  default     = "e1.1core-2ram"
}

variable "key_pair" {
  description = "Name of the key pair for SSH access"
  default = "terraform_ssh"
}

variable "network_id" {
  description = "ID of the network to attach the VM"
  default = "internal-ipv4-general-private"
}

variable "floating_ip_pool" {
  description = "Floating IP pool name"
  default = "external-ipv4-general-public"
}

# Security group
resource "openstack_networking_secgroup_v2" "sg" {
  name        = "homework-sg"
  description = "Allow web and SSH traffic"
}

# HTTP rule
resource "openstack_networking_secgroup_rule_v2" "http" {
  direction         = "ingress"
  ethertype         = "IPv4"
  protocol          = "tcp"
  port_range_min    = 80
  port_range_max    = 80
  remote_ip_prefix  = "0.0.0.0/0"
  security_group_id = openstack_networking_secgroup_v2.sg.id
}

# SSH rule
resource "openstack_networking_secgroup_rule_v2" "ssh" {
  direction         = "ingress"
  ethertype         = "IPv4"
  protocol          = "tcp"
  port_range_min    = 22
  port_range_max    = 22
  remote_ip_prefix  = "0.0.0.0/0"
  security_group_id = openstack_networking_secgroup_v2.sg.id
}

# Get network details
data "openstack_networking_network_v2" "internal" {
  name = var.network_id
}

data "openstack_networking_network_v2" "external" {
  name = var.floating_ip_pool
}

# Create VM
resource "openstack_compute_instance_v2" "vm" {
  name            = var.vm_name
  image_id        = var.image_id
  flavor_name     = "e1.tiny"
  key_pair        = var.key_pair
  security_groups = [openstack_networking_secgroup_v2.sg.name]

  network {
    name = var.network_id
  }

  user_data = <<-EOF
    #!/bin/bash
    apt-get update -y
    apt-get install apache2 -y
    echo "Name: Andrii Bezrukov, uco: 570943" > /var/www/html/index.html
    systemctl restart apache2
  EOF
}

output "internal_ip" {
  value = openstack_compute_instance_v2.vm.access_ip_v4
}
