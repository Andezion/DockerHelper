#!/bin/bash

get_running_containers() {
    docker ps --format "{{.ID}}|{{.Names}}|{{.Status}}|{{.Image}}" 2>/dev/null
}

get_docker_stats() {
    docker stats --no-stream --format "{{.Container}}|{{.CPUPerc}}|{{.MemUsage}}" 2>/dev/null
}

get_system_info() {
    local total_cpu=0
    local total_mem=0
    local count=0
    
    while IFS='|' read -r container cpu mem; do
        if [ -n "$cpu" ]; then
            cpu_val=$(echo "$cpu" | tr -d '%')
            total_cpu=$(echo "$total_cpu + $cpu_val" | bc 2>/dev/null || echo "0")
            count=$((count + 1))
        fi
    done < <(docker stats --no-stream --format "{{.Container}}|{{.CPUPerc}}|{{.MemUsage}}" 2>/dev/null)
    
    total_mem=$(docker stats --no-stream --format "{{.MemUsage}}" 2>/dev/null | awk '{sum+=$1} END {print sum}')
    
    echo "CPU:${total_cpu}%|MEM:${total_mem}|CONTAINERS:${count}"
}

stop_container() {
    local container_id=$1
    docker stop "$container_id" 2>/dev/null
    echo $?
}

get_unused_images() {
    docker images -f "dangling=true" --format "{{.ID}}|{{.Repository}}|{{.Tag}}|{{.Size}}" 2>/dev/null
}

get_stopped_containers() {
    docker ps -a --filter "status=exited" --format "{{.ID}}|{{.Names}}|{{.Status}}|{{.Image}}" 2>/dev/null
}

get_unused_volumes() {
    docker volume ls -f "dangling=true" --format "{{.Name}}|{{.Driver}}" 2>/dev/null
}

remove_container() {
    local container_id=$1
    docker rm "$container_id" 2>/dev/null
    echo $?
}

remove_image() {
    local image_id=$1
    docker rmi "$image_id" 2>/dev/null
    echo $?
}

remove_volume() {
    local volume_name=$1
    docker volume rm "$volume_name" 2>/dev/null
    echo $?
}

docker_prune_all() {
    docker system prune -af --volumes 2>/dev/null
    echo $?
}

get_reclaimable_space() {
    docker system df 2>/dev/null | tail -n 1 | awk '{print $NF}'
}

case "$1" in
    running)
        get_running_containers
        ;;
    stats)
        get_docker_stats
        ;;
    sysinfo)
        get_system_info
        ;;
    stop)
        stop_container "$2"
        ;;
    unused_images)
        get_unused_images
        ;;
    stopped)
        get_stopped_containers
        ;;
    unused_volumes)
        get_unused_volumes
        ;;
    remove_container)
        remove_container "$2"
        ;;
    remove_image)
        remove_image "$2"
        ;;
    remove_volume)
        remove_volume "$2"
        ;;
    prune)
        docker_prune_all
        ;;
    reclaimable)
        get_reclaimable_space
        ;;
    *)
        echo "Usage: $0 {running|stats|sysinfo|stop|unused_images|stopped|unused_volumes|remove_container|remove_image|remove_volume|prune|reclaimable} [id]"
        exit 1
        ;;
esac
